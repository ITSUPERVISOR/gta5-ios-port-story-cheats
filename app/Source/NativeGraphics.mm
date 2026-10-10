#import "NativeAssetServer.h"
#import <Foundation/Foundation.h>
#include <TargetConditionals.h>
#import <JavaScriptCore/JavaScriptCore.h>
#import <QuartzCore/CAMetalLayer.h>
#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#include <dawn/webgpu.h>
#include <dawn/native/MetalBackend.h>
#include "NativeGraphics.h"
#include "BCTextureDecoder.h"
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <atomic>
#include <algorithm>
#include <os/proc.h>
#include <mach/mach.h>
#include <notify.h>
#include <cmath>
#include <string>
#include <unordered_map>
#include "NativeFrameRate.h"
extern "C" void pg_log(const char *);
extern "C" uint32_t PGGraphicsNotify(uint64_t);
extern "C" uint64_t PGMemoryBudgetClamps(void);
extern "C" uint64_t PGMemoryBudgetQueries(void);

static JSContext *graphicsContext;
static dispatch_queue_t graphicsQueue;
static CAMetalLayer *metalLayer;
static WGPUInstance instance;
static WGPUAdapter adapter;
static WGPUDevice gpuDevice;
static WGPUQueue textureUploadQueue;
static WGPUSurface surface;
static bool surfacePending;
static JSValue *deviceObject;
static dispatch_source_t graphicsPump;
static double memoryReportTime;
static void *engineMemory;
static uint64_t engineMemorySize;
static double nativeBridgeMs,nativeBatchMs,nativeUploadMs;
static uint64_t nativeCallCount,nativeBatchCommandCount,nativePresentCount;
static PGFrameRateWindow hudFrameRate;
static std::atomic<double> hudFPS{0},hudLastPresent{0};
extern "C" double PGRenderedFramesPerSecond(void){
 double last=hudLastPresent.load(std::memory_order_relaxed);
 return last&&CACurrentMediaTime()-last<1.5?hudFPS.load(std::memory_order_relaxed):0;
}
static bool nativeOperationProfile;
static bool referenceTextureUploads,referenceBCDecoder;
static void (*nativeDetailProfileReport)();
struct NativeOperationStat {double ms=0;uint64_t calls=0;};
static std::unordered_map<std::string,NativeOperationStat> nativeOperations;
struct NativeCallClock {
 double start; bool upload;const char *operation;
 NativeCallClock(bool isUpload,const char *label=nullptr):start(CACurrentMediaTime()),upload(isUpload),operation(label){nativeCallCount++;}
 ~NativeCallClock(){double ms=(CACurrentMediaTime()-start)*1000;nativeBridgeMs+=ms;if(upload)nativeUploadMs+=ms;if(nativeOperationProfile&&operation){auto &stat=nativeOperations[operation];stat.ms+=ms;stat.calls++;}}
};
// Sub-operation timings do not add to the bridge total: the outer GPU call
// already includes them. They separate deferred Metal encoding from presentation.
struct NativeOperationClock {
 const char *operation;double start;
 explicit NativeOperationClock(const char *label):operation(label),start(nativeOperationProfile?CACurrentMediaTime():0){}
 ~NativeOperationClock(){if(start){auto &stat=nativeOperations[operation];stat.ms+=(CACurrentMediaTime()-start)*1000;stat.calls++;}}
};
static void reportNativeOperations(){
 std::vector<std::pair<std::string,NativeOperationStat>> entries(nativeOperations.begin(),nativeOperations.end());
 std::sort(entries.begin(),entries.end(),[](const auto &a,const auto &b){return a.second.ms>b.second.ms;});
 NSMutableString *line=[NSMutableString stringWithString:@"Native operation costs per frame:"];
 for(size_t i=0;i<std::min((size_t)6,entries.size());i++)[line appendFormat:@" %s %.2f ms (%llu calls);",entries[i].first.c_str(),entries[i].second.ms/nativePresentCount,entries[i].second.calls/nativePresentCount];
 pg_log(line.UTF8String);nativeOperations.clear();if(nativeDetailProfileReport)nativeDetailProfileReport();
}
static bool present(JSValue *v){return v&&!v.isUndefined&&!v.isNull;}
struct Arena {
 std::vector<void *> blocks;
 ~Arena(){for(void *p:blocks)free(p);}
 template<class T>T *alloc(size_t count=1){T *p=(T *)calloc(std::max((size_t)1,count),sizeof(T));blocks.push_back(p);return p;}
 WGPUStringView string(JSValue *v){if(!present(v))return WGPU_STRING_VIEW_INIT;const char *text=v.toString.UTF8String;char *p=strdup(text);blocks.push_back(p);return {p,strlen(p)};}
};
struct Capsule {void *handle;void(*release)(void *);};
static void finalizeGPU(JSObjectRef object){Capsule *p=(Capsule *)JSObjectGetPrivate(object);if(p){if(p->handle&&p->release)p->release(p->handle);delete p;}}
static JSClassRef capsuleClass(){static JSClassRef result;static dispatch_once_t once;dispatch_once(&once,^{JSClassDefinition d=kJSClassDefinitionEmpty;d.className="NativeGPUHandle";d.finalize=finalizeGPU;result=JSClassCreate(&d);});return result;}
static void *rawGPUHandle(JSContextRef c,JSValueRef value) {
 if(!JSValueIsObject(c,value))return nullptr;
 static JSStringRef key=JSStringCreateWithUTF8CString("_native");
 JSValueRef wrapped=JSObjectGetProperty(c,JSValueToObject(c,value,nullptr),key,nullptr);
 if(!JSValueIsObject(c,wrapped))return nullptr;
 Capsule *capsule=(Capsule *)JSObjectGetPrivate(JSValueToObject(c,wrapped,nullptr));return capsule?capsule->handle:nullptr;
}
static void *gpuHandle(JSValue *value){return value?rawGPUHandle(value.context.JSGlobalContextRef,value.JSValueRef):nullptr;}
static JSValue *wrapGPU(JSContext *context,NSString *kind,void *handle,void(*release)(void *)) {
 if(!handle)return [JSValue valueWithNullInContext:context];
 JSObjectRef object=JSObjectMake(context.JSGlobalContextRef,capsuleClass(),new Capsule{handle,release});
 JSValue *capsule=[JSValue valueWithJSValueRef:object inContext:context];
 JSValue *value=[context[@"wrapNativeGPU"] callWithArguments:@[kind,capsule]];
 if([kind isEqualToString:@"buffer"])value[@"size"]=@(wgpuBufferGetSize((WGPUBuffer)handle));
 return value;
}
static JSValue *normalizeVector(JSValue *value,NSString *kind){return [value.context[@"normalizeGPUVector"] callWithArguments:@[value?:[JSValue valueWithUndefinedInContext:graphicsContext],kind]];}
static JSValue *normalizeBinding(JSValue *value){return [value.context[@"normalizeGPUBinding"] callWithArguments:@[value]];}
#include "GeneratedGPUBridge.inc"

static JSValue *bytesValue(NSData *data) {
 void *buffer=malloc(std::max((NSUInteger)1,data.length));memcpy(buffer,data.bytes,data.length);
 JSObjectRef object=JSObjectMakeArrayBufferWithBytesNoCopy(graphicsContext.JSGlobalContextRef,buffer,data.length,[](void *p,void *){free(p);},nullptr,nullptr);
 return [JSValue valueWithJSValueRef:object inContext:graphicsContext];
}
static NSData *dataValue(JSValue *v,size_t start=0,size_t length=SIZE_MAX) {
 JSValue *buffer=present(v[@"buffer"])?v[@"buffer"]:v;
 JSObjectRef object=JSValueToObject(v.context.JSGlobalContextRef,buffer.JSValueRef,nullptr);
 size_t total=JSObjectGetArrayBufferByteLength(v.context.JSGlobalContextRef,object,nullptr);
 uint8_t *bytes=(uint8_t *)JSObjectGetArrayBufferBytesPtr(v.context.JSGlobalContextRef,object,nullptr);
 size_t offset=present(v[@"byteOffset"])?v[@"byteOffset"].toDouble:0;
 size_t size=present(v[@"byteLength"])?v[@"byteLength"].toDouble:total;
 if(offset>total||size>total-offset||start>size)return nil;
 size=std::min(length,size-start);return [NSData dataWithBytes:bytes+offset+start length:size];
}
// Borrowed bytes are consumed synchronously. The final JSC call obtains the
// pointer; callers must not call JSC again until decoding/upload has finished.
#include "NativeTextureView.h"
#include "NativeTextureRows.h"
#include "NativeUniforms.inc"

// Read upload views through the JSC C API. This preserves typed-array element
// sizes and avoids copying every upload through an Objective-C data object.
#include "NativeDrawKernel.inc"
#include "NativeRenderPass.inc"
#include "NativePipelineCache.inc"
#include "NativePreparedDraws.inc"
#include "NativeWarmDispatch.inc"
#include "NativeGPUBatch.inc"
#include "NativeShaderCache.inc"
static JSValueRef storeTextureRowsDirect(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 NativeCallClock timing(true,nativeOperationProfile?"texture.shadowRows":nullptr);
 if(!PGStoreTextureRows(c,n,v,engineMemory,engineMemorySize,error)){
  if(!*error){JSStringRef message=JSStringCreateWithUTF8CString("Invalid compressed texture upload range");JSValueRef text=JSValueMakeString(c,message);JSStringRelease(message);*error=JSObjectMakeError(c,1,&text,nullptr);}
 }
 return JSValueMakeUndefined(c);
}
static JSValueRef writeBufferDirect(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error) {
 NativeCallClock timing(true,"queue.writeBuffer");
 auto fail=[&](){JSStringRef s=JSStringCreateWithUTF8CString("Invalid native GPU writeBuffer range");JSValueRef message=JSValueMakeString(c,s);JSStringRelease(s);*error=JSObjectMakeError(c,1,&message,nullptr);return JSValueMakeUndefined(c);};
 if(n<4)return fail();
 WGPUQueue queue=(WGPUQueue)rawGPUHandle(c,v[0]);WGPUBuffer buffer=(WGPUBuffer)rawGPUHandle(c,v[1]);
 double destination=JSValueToNumber(c,v[2],error);
 JSObjectRef input=JSValueToObject(c,v[3],error);JSTypedArrayType type=JSValueGetTypedArrayType(c,v[3],error);
 size_t byteOffset=0,bytes=0,element=1;JSObjectRef backing=nullptr;
 if(type==kJSTypedArrayTypeArrayBuffer){backing=input;bytes=JSObjectGetArrayBufferByteLength(c,backing,error);}
 else if(type!=kJSTypedArrayTypeNone){bytes=JSObjectGetTypedArrayByteLength(c,input,error);size_t elements=JSObjectGetTypedArrayLength(c,input,error);if(elements)element=bytes/elements;byteOffset=JSObjectGetTypedArrayByteOffset(c,input,error);backing=JSObjectGetTypedArrayBuffer(c,input,error);}
 else return fail();
 double start=n>4&&!JSValueIsUndefined(c,v[4])?JSValueToNumber(c,v[4],error):0;
 double count=n>5&&!JSValueIsUndefined(c,v[5])?JSValueToNumber(c,v[5],error):(double)bytes/element-start;
 if(!queue||!buffer||!backing||!std::isfinite(destination)||destination<0||floor(destination)!=destination||!std::isfinite(start)||start<0||floor(start)!=start||!std::isfinite(count)||count<0||floor(count)!=count||start>bytes/element||count>bytes/element-start)return fail();
 size_t length=(size_t)count*element,source=(size_t)start*element;
 if(((uint64_t)destination%4)||length%4)return fail();
 const uint8_t *data=(const uint8_t *)JSObjectGetArrayBufferBytesPtr(c,backing,error);
 if(length&&!data)return fail();
 wgpuQueueWriteBuffer(queue,buffer,(uint64_t)destination,data?data+byteOffset+source:nullptr,length);return JSValueMakeUndefined(c);
}
#include "NativeFrameReports.inc"
#if TARGET_OS_SIMULATOR
struct FrameSample {double time,interval;uint32_t draws;bool world;};
static std::vector<FrameSample> frameSamples;
struct TextureSample {double time,ms;size_t bytes;uint32_t format,width,height,depth;bool decode;};
static std::vector<TextureSample> textureSamples;
struct NativeTextureClock {
 double start;size_t bytes;uint32_t format,width,height,depth;bool decode;
 NativeTextureClock(bool decoding,size_t count,uint32_t kind,uint32_t w,uint32_t h,uint32_t d):start(nativeOperationProfile?NSProcessInfo.processInfo.systemUptime*1000:0),bytes(count),format(kind),width(w),height(h),depth(d),decode(decoding){}
 void setBytes(size_t count){bytes=count;}
 ~NativeTextureClock(){if(start&&textureSamples.size()<1000000)textureSamples.push_back({start,NSProcessInfo.processInfo.systemUptime*1000-start,bytes,format,width,height,depth,decode});}
};
static void saveTextureSamples(){
 if(!textureSamples.empty()){
  NSMutableString *csv=[NSMutableString stringWithString:@"start_ms,duration_ms,bytes,format,width,height,depth,decode\n"];
  for(const auto &t:textureSamples)[csv appendFormat:@"%.3f,%.3f,%zu,%u,%u,%u,%u,%u\n",t.time,t.ms,t.bytes,t.format,t.width,t.height,t.depth,t.decode];
  NSURL *url=[[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject URLByAppendingPathComponent:@"native-texture-timings.csv"];
  [csv writeToURL:url atomically:YES encoding:NSUTF8StringEncoding error:nil];
 }
}
static void saveFrameSamples(){
 NSMutableString *csv=[NSMutableString stringWithString:@"time_ms,frame_ms,draws,world\n"];for(const auto &f:frameSamples)[csv appendFormat:@"%.3f,%.3f,%u,%u\n",f.time,f.interval,f.draws,f.world];
 NSURL *url=[[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject URLByAppendingPathComponent:@"renderer-frames.csv"];
 [csv writeToURL:url atomically:YES encoding:NSUTF8StringEncoding error:nil];pg_log("Simulator frame timings saved.");
}
#else
struct NativeTextureClock {NativeTextureClock(bool,size_t,uint32_t,uint32_t,uint32_t,uint32_t){}void setBytes(size_t){}};
#endif

static JSValueRef recordFrame(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *) {
#if TARGET_OS_SIMULATOR
 if(n>=4&&frameSamples.size()<100000){double time=JSValueToNumber(c,v[0],nullptr),interval=JSValueToNumber(c,v[1],nullptr);if(std::isfinite(interval)&&interval>0)frameSamples.push_back({time,interval,(uint32_t)JSValueToNumber(c,v[2],nullptr),JSValueToBoolean(c,v[3])});}
#endif
 NativeFrameReports::record(c,n,v);return JSValueMakeUndefined(c);
}

static NSString *gpuString(WGPUStringView view){return [[NSString alloc] initWithBytes:view.data length:view.length==WGPU_STRLEN?strlen(view.data):view.length encoding:NSUTF8StringEncoding]?:@"Unknown GPU error";}
static JSValue *nativeCall(NSString *operation,JSValue *object,JSValue *args) {
 NativeCallClock timing([operation hasPrefix:@"queue.write"],nativeOperationProfile?operation.UTF8String:nullptr);
 JSContext *context=args.context;Arena a;
 if([operation isEqualToString:@"queue.writeBuffer"]){
  JSValue *input=[args valueAtIndex:2];size_t element=present(input[@"BYTES_PER_ELEMENT"])?input[@"BYTES_PER_ELEMENT"].toUInt32:1;
  size_t start=[args valueAtIndex:3].isUndefined?0:[args valueAtIndex:3].toDouble*element;
  size_t count=[args valueAtIndex:4].isUndefined?SIZE_MAX:[args valueAtIndex:4].toDouble*element;
  NSData *data=dataValue(input,start,count);if(!data){context.exception=[JSValue valueWithNewErrorFromMessage:@"Invalid native GPU writeBuffer range" inContext:context];return nil;}
  wgpuQueueWriteBuffer((WGPUQueue)gpuHandle(object),(WGPUBuffer)gpuHandle([args valueAtIndex:0]),[args valueAtIndex:1].toDouble,data.bytes,data.length);return nil;
 }
 if([operation isEqualToString:@"queue.writeTexture"]){
  auto target=readTexelCopyTextureInfo([args valueAtIndex:0],a);auto layout=readTexelCopyBufferLayout([args valueAtIndex:2],a);auto size=readExtent3D([args valueAtIndex:3],a);
  WGPUQueue queue=(WGPUQueue)gpuHandle(object);JSValue *input=[args valueAtIndex:1];
  NativeTextureClock textureClock(false,0,0,size.width,size.height,size.depthOrArrayLayers);
  if(referenceTextureUploads){NSData *data=dataValue(input);textureClock.setBytes(data.length);wgpuQueueWriteTexture(queue,&target,data.bytes,data.length,&layout,&size);}
  else{PGTextureView view;if(!PGGetTextureView(input.context.JSGlobalContextRef,input.JSValueRef,&view)){context.exception=[JSValue valueWithNewErrorFromMessage:@"Invalid native GPU writeTexture data" inContext:context];return nil;}textureClock.setBytes(view.length);wgpuQueueWriteTexture(queue,&target,view.bytes,view.length,&layout,&size);}
  return nil;
 }
 if([operation isEqualToString:@"queue.submit"]){JSValue *list=[args valueAtIndex:0];size_t count=list[@"length"].toUInt32;std::vector<WGPUCommandBuffer> buffers;for(size_t i=0;i<count;i++)buffers.push_back((WGPUCommandBuffer)gpuHandle([list valueAtIndex:i]));{NativeOperationClock timing("gpu.encodeSubmit");wgpuQueueSubmit((WGPUQueue)gpuHandle(object),count,buffers.data());}if(surfacePending){NativeOperationClock timing("gpu.present");wgpuSurfacePresent(surface);double presentedAt=CACurrentMediaTime();hudLastPresent.store(presentedAt,std::memory_order_relaxed);if(hudFrameRate.record(presentedAt))hudFPS.store(hudFrameRate.rate,std::memory_order_relaxed);surfacePending=false;}return nil;}
 if([operation isEqualToString:@"buffer.getMappedRange"]){WGPUBuffer buffer=(WGPUBuffer)gpuHandle(object);size_t offset=[args valueAtIndex:0].isUndefined?0:[args valueAtIndex:0].toDouble;size_t size=[args valueAtIndex:1].isUndefined?wgpuBufferGetSize(buffer)-offset:[args valueAtIndex:1].toDouble;const void *p=wgpuBufferGetConstMappedRange(buffer,offset,size);if(!p){context.exception=[JSValue valueWithNewErrorFromMessage:@"Native GPU buffer is not mapped" inContext:context];return nil;}return bytesValue([NSData dataWithBytes:p length:size]);}
 if([operation isEqualToString:@"canvas.configure"]){auto config=WGPU_SURFACE_CONFIGURATION_INIT;config.device=gpuDevice;config.format=WGPUTextureFormat_BGRA8Unorm;config.usage=WGPUTextureUsage_RenderAttachment;config.width=object[@"width"].toUInt32;config.height=object[@"height"].toUInt32;config.presentMode=WGPUPresentMode_Fifo;config.alphaMode=WGPUCompositeAlphaMode_Opaque;wgpuSurfaceConfigure(surface,&config);return nil;}
 if([operation isEqualToString:@"canvas.getCurrentTexture"]){nativePresentCount++;auto frame=WGPU_SURFACE_TEXTURE_INIT;wgpuSurfaceGetCurrentTexture(surface,&frame);surfacePending=frame.texture!=nullptr;return wrapGPU(context,@"texture",frame.texture,[](void *p){wgpuTextureRelease((WGPUTexture)p);});}
 JSValue *result=gpuGeneratedCall(operation,object,args);
 if([operation isEqualToString:@"commandEncoder.beginRenderPass"])NativeDraw::invalidate();
 if(!result){context.exception=[JSValue valueWithNewErrorFromMessage:[@"Unknown native GPU call: " stringByAppendingString:operation] inContext:context];}
 return result;
}
struct AsyncGPU {__strong JSValue *resolve;__strong JSValue *reject;};
static void finishAsync(AsyncGPU *pending,bool success,NSString *message) {dispatch_async(graphicsQueue,^{@autoreleasepool {if(success)[pending->resolve callWithArguments:@[]];else [pending->reject callWithArguments:@[message?:@"GPU operation failed"]];delete pending;}});}
static void nativeAsync(NSString *operation,JSValue *object,JSValue *args,JSValue *resolve,JSValue *reject){
 AsyncGPU *pending=new AsyncGPU{resolve,reject};
 if([operation isEqualToString:@"buffer.mapAsync"]){auto callback=WGPU_BUFFER_MAP_CALLBACK_INFO_INIT;callback.mode=WGPUCallbackMode_AllowSpontaneous;callback.userdata1=pending;callback.callback=[](WGPUMapAsyncStatus status,WGPUStringView message,void *p,void *){finishAsync((AsyncGPU *)p,status==WGPUMapAsyncStatus_Success,gpuString(message));};WGPUBuffer buffer=(WGPUBuffer)gpuHandle(object);size_t offset=[args valueAtIndex:1].isUndefined?0:[args valueAtIndex:1].toDouble;size_t size=[args valueAtIndex:2].isUndefined?wgpuBufferGetSize(buffer)-offset:[args valueAtIndex:2].toDouble;wgpuBufferMapAsync(buffer,(WGPUMapMode)[args valueAtIndex:0].toUInt32,offset,size,callback);return;}
 if([operation isEqualToString:@"queue.onSubmittedWorkDone"]){auto callback=WGPU_QUEUE_WORK_DONE_CALLBACK_INFO_INIT;callback.mode=WGPUCallbackMode_AllowSpontaneous;callback.userdata1=pending;callback.callback=[](WGPUQueueWorkDoneStatus status,WGPUStringView message,void *p,void *){finishAsync((AsyncGPU *)p,status==WGPUQueueWorkDoneStatus_Success,gpuString(message));};wgpuQueueOnSubmittedWorkDone((WGPUQueue)gpuHandle(object),callback);return;}
 finishAsync(pending,false,@"Unknown native GPU async call");
}
static bool createMetalGPU(){
 auto descriptor=WGPU_INSTANCE_DESCRIPTOR_INIT;instance=wgpuCreateInstance(&descriptor);if(!instance)return false;
 struct AdapterResult {WGPUAdapter value;dispatch_semaphore_t done;};AdapterResult *result=new AdapterResult{nullptr,dispatch_semaphore_create(0)};
 auto options=WGPU_REQUEST_ADAPTER_OPTIONS_INIT;options.backendType=WGPUBackendType_Metal;options.powerPreference=WGPUPowerPreference_HighPerformance;
 auto callback=WGPU_REQUEST_ADAPTER_CALLBACK_INFO_INIT;callback.mode=WGPUCallbackMode_AllowSpontaneous;callback.userdata1=result;
 callback.callback=[](WGPURequestAdapterStatus status,WGPUAdapter value,WGPUStringView message,void *p,void *){auto *r=(AdapterResult *)p;r->value=value;if(status!=WGPURequestAdapterStatus_Success)pg_log(gpuString(message).UTF8String);dispatch_semaphore_signal(r->done);};
 wgpuInstanceRequestAdapter(instance,&options,callback);dispatch_semaphore_wait(result->done,DISPATCH_TIME_FOREVER);adapter=result->value;delete result;if(!adapter)return false;
 auto source=WGPU_SURFACE_SOURCE_METAL_LAYER_INIT;source.layer=(__bridge void *)metalLayer;
 auto surfaceDescriptor=WGPU_SURFACE_DESCRIPTOR_INIT;surfaceDescriptor.nextInChain=&source.chain;surface=wgpuInstanceCreateSurface(instance,&surfaceDescriptor);return surface!=nullptr;
}
static JSValue *requestDevice(JSValue *requirements) {
 Arena a;auto descriptor=WGPU_DEVICE_DESCRIPTOR_INIT;auto limits=WGPU_LIMITS_INIT;
 auto cache=ShaderCache::descriptor();descriptor.nextInChain=&cache.chain;
 auto toggles=WGPU_DAWN_TOGGLES_DESCRIPTOR_INIT;const char *fastToggles[]={"skip_validation"};
 if([NSProcessInfo.processInfo.arguments containsObject:@"--fast-renderer"]){toggles.enabledToggleCount=1;toggles.enabledToggles=fastToggles;toggles.chain.next=descriptor.nextInChain;descriptor.nextInChain=&toggles.chain;pg_log("GPU command validation disabled; shader buffer robustness remains enabled.");}
 else pg_log("GPU command validation enabled.");
 JSValue *features=requirements[@"requiredFeatures"];std::vector<WGPUFeatureName> requested;
 // The wrapper lists only features returned by this adapter.
 NSDictionary *names=graphicsContext[@"nativeFeatureNames"].toDictionary;
 for(NSString *feature in features.toArray)if(names[feature])requested.push_back((WGPUFeatureName)[names[feature] unsignedIntValue]);
 descriptor.requiredFeatures=requested.data();descriptor.requiredFeatureCount=requested.size();
 limits=readLimits(requirements[@"requiredLimits"],a);descriptor.requiredLimits=&limits;
 descriptor.uncapturedErrorCallbackInfo.callback=[](WGPUDevice const *,WGPUErrorType,WGPUStringView message,void *,void *){NSString *s=gpuString(message);dispatch_async(graphicsQueue,^{@autoreleasepool {[graphicsContext[@"nativeGPUError"] callWithArguments:@[s]];}});};
 struct DeviceResult {WGPUDevice value;dispatch_semaphore_t done;};DeviceResult *result=new DeviceResult{nullptr,dispatch_semaphore_create(0)};
 auto callback=WGPU_REQUEST_DEVICE_CALLBACK_INFO_INIT;callback.mode=WGPUCallbackMode_AllowSpontaneous;callback.userdata1=result;
 callback.callback=[](WGPURequestDeviceStatus status,WGPUDevice value,WGPUStringView message,void *p,void *){auto *r=(DeviceResult *)p;r->value=value;if(status!=WGPURequestDeviceStatus_Success)pg_log(gpuString(message).UTF8String);dispatch_semaphore_signal(r->done);};
 wgpuAdapterRequestDevice(adapter,&descriptor,callback);dispatch_semaphore_wait(result->done,DISPATCH_TIME_FOREVER);gpuDevice=result->value;delete result;if(!gpuDevice)return [JSValue valueWithNullInContext:graphicsContext];
 wgpuDeviceAddRef(gpuDevice);deviceObject=wrapGPU(graphicsContext,@"device",gpuDevice,[](void *p){wgpuDeviceRelease((WGPUDevice)p);});
 deviceObject[@"limits"]=graphicsContext[@"nativeLimits"];
 WGPUQueue queue=wgpuDeviceGetQueue(gpuDevice);textureUploadQueue=queue;deviceObject[@"queue"]=wrapGPU(graphicsContext,@"queue",queue,[](void *p){wgpuQueueRelease((WGPUQueue)p);});return deviceObject;
}
static void installRuntime(void *memory,uint64_t size){
 graphicsContext=[[JSContext alloc] init];graphicsContext.exceptionHandler=^(JSContext *,JSValue *e){pg_log([@"Native renderer exception: " stringByAppendingString:e.toString].UTF8String);};
#if TARGET_OS_SIMULATOR
 graphicsContext[@"nativeGPUDescription"]=@"Simulator native Metal (A15 texture paths)";
#else
 graphicsContext[@"nativeGPUDescription"]=@"iPhone native Metal";
#endif
 bool metalBufferOffsets=![NSProcessInfo.processInfo.arguments containsObject:@"--reference-metal-offsets"];
 if(metalBufferOffsets)unsetenv("PG_REFERENCE_METAL_BUFFER_OFFSETS");else setenv("PG_REFERENCE_METAL_BUFFER_OFFSETS","1",1);
 graphicsContext[@"nativeMetalBufferOffsets"]=@(metalBufferOffsets);
 NativeUniforms::install(graphicsContext);
 referenceTextureUploads=[NSProcessInfo.processInfo.arguments containsObject:@"--reference-texture-uploads"];
 referenceBCDecoder=[NSProcessInfo.processInfo.arguments containsObject:@"--reference-bc-decoder"];
 bool fusedBCUploads=!referenceTextureUploads&&![NSProcessInfo.processInfo.arguments containsObject:@"--reference-fused-texture-uploads"];
 graphicsContext[@"nativeFusedBCUploads"]=@(fusedBCUploads);
 pg_log([[NSString stringWithFormat:@"Native texture paths: uploads %@; BC decoder %@; fused BC %@; native shadow rows %@.",referenceTextureUploads?@"copied":@"borrowed",referenceBCDecoder?@"block-copy":@"direct",fusedBCUploads?@"on":@"off",[NSProcessInfo.processInfo.arguments containsObject:@"--reference-texture-shadows"]?@"off":@"on"] UTF8String]);
 nativeOperationProfile=[NSProcessInfo.processInfo.arguments containsObject:@"--profile"]||[NSProcessInfo.processInfo.arguments containsObject:@"--profile-bridge"];
 graphicsContext[@"nativeBridgeProfile"]=@(nativeOperationProfile);
 NativeDraw::install(graphicsContext);
 NativePass::install(graphicsContext);
 NativePipeline::install(graphicsContext);
 NativePreparedDraw::install(graphicsContext);
 NativeWarm::install(graphicsContext);
 NativeUniforms::expose(graphicsContext.JSGlobalContextRef,"nativeWriteBuffer",writeBufferDirect);
 graphicsContext[@"nativeTextureShadowRows"]=@(![NSProcessInfo.processInfo.arguments containsObject:@"--reference-texture-shadows"]);
 NativeUniforms::expose(graphicsContext.JSGlobalContextRef,"nativeStoreTextureRows",storeTextureRowsDirect);
 NativeFrameReports::memoryClampSample=PGMemoryBudgetClamps;NativeFrameReports::gpuMemorySample=[]()->uint64_t{return gpuDevice?dawn::native::metal::GetMTLDevice(gpuDevice).currentAllocatedSize:0;};NativeFrameReports::install(graphicsContext);
 NativeUniforms::expose(graphicsContext.JSGlobalContextRef,"nativeRecordFrame",recordFrame);
 graphicsContext[@"nativeLog"]=^(NSString *s){pg_log(s.UTF8String);};
 // Small renderer state files (e.g. the pipeline recipes seen in play) in
 // Documents/RendererState: JavaScriptCore has no IndexedDB.
 graphicsContext[@"nativeReadText"]=^JSValue *(NSString *name){
  NSString *folder=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:@"RendererState"];
  NSString *text=[NSString stringWithContentsOfFile:[folder stringByAppendingPathComponent:name.lastPathComponent] encoding:NSUTF8StringEncoding error:nil];
  return text?[JSValue valueWithObject:text inContext:graphicsContext]:[JSValue valueWithNullInContext:graphicsContext];
 };
 // Read-only files shipped in the app bundle (e.g. pipeline recipes recorded in
 // the simulator, so even a first launch compiles them while loading).
 graphicsContext[@"nativeReadBundleText"]=^JSValue *(NSString *name){
  NSString *path=[NSBundle.mainBundle pathForResource:name.lastPathComponent.stringByDeletingPathExtension ofType:name.pathExtension];
  NSString *text=path?[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil]:nil;
  return text?[JSValue valueWithObject:text inContext:graphicsContext]:[JSValue valueWithNullInContext:graphicsContext];
 };
 // Appends only, so a save costs as much as the new lines, never the whole file.
 graphicsContext[@"nativeAppendText"]=^BOOL(NSString *name,NSString *text){
  NSString *folder=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:@"RendererState"];
  [NSFileManager.defaultManager createDirectoryAtPath:folder withIntermediateDirectories:YES attributes:nil error:nil];
  NSString *path=[folder stringByAppendingPathComponent:name.lastPathComponent];
  if(![NSFileManager.defaultManager fileExistsAtPath:path])[NSData.data writeToFile:path atomically:YES];
  NSFileHandle *handle=[NSFileHandle fileHandleForWritingAtPath:path];
  if(!handle)return NO;
  [handle seekToEndOfFile];[handle writeData:[text dataUsingEncoding:NSUTF8StringEncoding]];[handle closeFile];
  return YES;
 };
 NativeUniforms::expose(graphicsContext.JSGlobalContextRef,"nativeGPUBatch",nativeBatch);
 graphicsContext[@"nativeGPUCall"]=^JSValue *(NSString *op,JSValue *object,JSValue *args){return nativeCall(op,object,args);};
 graphicsContext[@"nativeGPUAsync"]=^(NSString *op,JSValue *object,JSValue *args,JSValue *resolve,JSValue *reject){nativeAsync(op,object,args,resolve,reject);};
 graphicsContext[@"nativeRequestDevice"]=^JSValue *(JSValue *requirements){return requestDevice(requirements);};
 graphicsContext[@"nativeLater"]=^(JSValue *callback,double delay){dispatch_after(dispatch_time(DISPATCH_TIME_NOW,(int64_t)(std::max(0.0,delay)*1000000)),graphicsQueue,^{@autoreleasepool {[callback callWithArguments:@[]];}});};
 // Decode and upload in one native call. Dawn consumes the source before
 // returning, so temporary expanded pixels do not need a GC-owned JS buffer.
 graphicsContext[@"nativeUploadBC"]=^(JSValue *destination,NSString *format,uint32_t width,uint32_t height,uint32_t depth,uint32_t pitch,uint32_t rows,JSValue *data){
  NativeCallClock timing(true,nativeOperationProfile?"texture.decodeUploadBC":nullptr);Arena arena;
  auto target=readTexelCopyTextureInfo(destination,arena);
  int kind=format.length>=3?[[format substringWithRange:NSMakeRange(2,1)] intValue]:0;
  size_t decodedSize=0;void *decoded=nullptr;
  {
   NativeTextureClock textureClock(true,0,kind,width,height,depth);
   PGTextureView view;if(PGGetTextureView(data.context.JSGlobalContextRef,data.JSValueRef,&view)){
    textureClock.setBytes(view.length);decoded=(referenceBCDecoder?PGDecodeBCReference:PGDecodeBC)(kind,width,height,depth,pitch,rows,view.bytes,view.length,&decodedSize);
   }
  }
  if(!decoded){graphicsContext.exception=[JSValue valueWithNewErrorFromMessage:@"Invalid native BC texture data" inContext:graphicsContext];return;}
  size_t pixelBytes=kind==4?1:kind==5?2:kind==6?8:4;
  auto layout=WGPU_TEXEL_COPY_BUFFER_LAYOUT_INIT;layout.bytesPerRow=width*(uint32_t)pixelBytes;layout.rowsPerImage=height;
  auto extent=WGPU_EXTENT_3D_INIT;extent.width=width;extent.height=height;extent.depthOrArrayLayers=depth;
  {NativeTextureClock textureClock(false,decodedSize,kind,width,height,depth);wgpuQueueWriteTexture(textureUploadQueue,&target,decoded,decodedSize,&layout,&extent);}
  free(decoded);
 };
 graphicsContext[@"nativeDecodeBC"]=^JSValue *(NSString *format,uint32_t width,uint32_t height,uint32_t depth,uint32_t pitch,uint32_t rows,JSValue *data){
  NativeCallClock timing(true,nativeOperationProfile?"texture.decodeBC":nullptr);
  size_t size=0;int kind=format.length>=3?[[format substringWithRange:NSMakeRange(2,1)] intValue]:0;
  NativeTextureClock textureClock(true,0,kind,width,height,depth);
  void *decoded=nullptr;
  auto decoder=referenceBCDecoder?PGDecodeBCReference:PGDecodeBC;
  if(referenceTextureUploads){NSData *source=dataValue(data);textureClock.setBytes(source.length);decoded=decoder(kind,width,height,depth,pitch,rows,source.bytes,source.length,&size);}
  else{PGTextureView view;if(PGGetTextureView(data.context.JSGlobalContextRef,data.JSValueRef,&view)){textureClock.setBytes(view.length);decoded=decoder(kind,width,height,depth,pitch,rows,view.bytes,view.length,&size);}}
  if(!decoded){graphicsContext.exception=[JSValue valueWithNewErrorFromMessage:@"Invalid native BC texture data" inContext:graphicsContext];return nil;}
  JSObjectRef buffer=JSObjectMakeArrayBufferWithBytesNoCopy(graphicsContext.JSGlobalContextRef,decoded,size,[](void *p,void *){free(p);},nullptr,nullptr);
  return [JSValue valueWithJSValueRef:buffer inContext:graphicsContext];
 };
 graphicsContext[@"nativeSaveShot"]=^(NSString *url,JSValue *pixels){
  NSURLComponents *parts=[NSURLComponents componentsWithString:[PGAssetServer() stringByAppendingString:url]];
  NSMutableDictionary *parameters=[NSMutableDictionary dictionary];for(NSURLQueryItem *item in parts.queryItems)parameters[item.name]=item.value?:@"";
  size_t width=[parameters[@"w"] integerValue],height=[parameters[@"h"] integerValue];NSData *bytes=dataValue(pixels);
  if(!width||!height||width>8192||height>8192||bytes.length!=width*height*4)return;
  CGColorSpaceRef color=CGColorSpaceCreateDeviceRGB();CGContextRef bitmap=CGBitmapContextCreate((void *)bytes.bytes,width,height,8,width*4,color,kCGBitmapByteOrder32Big|kCGImageAlphaPremultipliedLast);
  CGImageRef image=CGBitmapContextCreateImage(bitmap);NSData *png=UIImagePNGRepresentation([UIImage imageWithCGImage:image]);
  NSString *name=[parameters[@"name"] lastPathComponent];NSString *folder=[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:@"NativeFrames"];
  [[NSFileManager defaultManager] createDirectoryAtPath:folder withIntermediateDirectories:YES attributes:nil error:nil];
  [png writeToFile:[[folder stringByAppendingPathComponent:name?:@"frame"] stringByAppendingPathExtension:@"png"] atomically:YES];
  pg_log([@"Native rendered frame saved: " stringByAppendingString:name?:@"frame"].UTF8String);
  CGImageRelease(image);CGContextRelease(bitmap);CGColorSpaceRelease(color);
 };
 graphicsContext[@"nativeFetch"]=^(NSString *url,JSValue *options,JSValue *resolve,JSValue *reject){
  NSURL *target=[NSURL URLWithString:url relativeToURL:[NSURL URLWithString:[PGAssetServer() stringByAppendingString:@"/"]]];
  NSMutableURLRequest *request=[NSMutableURLRequest requestWithURL:target];NSDictionary *headers=options[@"headers"].toDictionary;for(NSString *key in headers)[request setValue:[headers[key] description] forHTTPHeaderField:key];
  [[NSURLSession.sharedSession dataTaskWithRequest:request completionHandler:^(NSData *data,NSURLResponse *reply,NSError *error){dispatch_async(graphicsQueue,^{@autoreleasepool {if(error){[reject callWithArguments:@[error.localizedDescription]];return;}NSHTTPURLResponse *response=(NSHTTPURLResponse *)reply;[resolve callWithArguments:@[@(response.statusCode),bytesValue(data),response.allHeaderFields]];}});}] resume];
 };
 graphicsContext[@"nativeAtomic"]=^double(NSString *op,double address,double value){uint64_t at=(uint64_t)address;if(at>size-4||(at&3)){pg_log("Invalid renderer atomic address.");return 0; }auto *p=(std::atomic<int32_t> *)((uint8_t *)memory+at);if([op isEqualToString:@"load"])return p->load();if([op isEqualToString:@"store"]){p->store((int32_t)value);return value;}if([op isEqualToString:@"add"])return p->fetch_add((int32_t)value);if([op isEqualToString:@"exchange"])return p->exchange((int32_t)value);return 0;};
 graphicsContext[@"nativeNotify"]=^(double at){return PGGraphicsNotify(at);};
 graphicsContext[@"nativeDecode"]=^NSString *(JSValue *bytes){NSData *data=dataValue(bytes);return [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding]?:@"";};
 graphicsContext[@"nativeEncode"]=^JSValue *(NSString *text){return bytesValue([text dataUsingEncoding:NSUTF8StringEncoding]);};
 graphicsContext[@"nativeNow"]=^double{return NSProcessInfo.processInfo.systemUptime*1000;};
 JSObjectRef buffer=JSObjectMakeArrayBufferWithBytesNoCopy(graphicsContext.JSGlobalContextRef,memory,size,[](void *,void *){},nullptr,nullptr);graphicsContext[@"nativeMemory"]=[JSValue valueWithJSValueRef:buffer inContext:graphicsContext];
 graphicsContext[@"nativeAssetBase"]=PGAssetServer();
 for(NSString *name in @[@"gpu-runtime",@"gpu-methods",@"gpu-enums",@"gpu-batch"]) {NSString *path=[NSBundle.mainBundle pathForResource:name ofType:@"js"];NSString *source=[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];[graphicsContext evaluateScript:source withSourceURL:[NSURL fileURLWithPath:path]];}
 auto limits=WGPU_LIMITS_INIT;wgpuAdapterGetLimits(adapter,&limits);
 NSMutableDictionary *serialized=[NSMutableDictionary dictionary];
#include "GeneratedGPULimits.inc"
 graphicsContext[@"nativeLimits"]=serialized;
 NSMutableArray *features=[NSMutableArray array];NSDictionary *names=graphicsContext[@"nativeFeatureNames"].toDictionary;
 for(NSString *name in names)if(wgpuAdapterHasFeature(adapter,(WGPUFeatureName)[names[name] unsignedIntValue]))[features addObject:name];
#if TARGET_OS_SIMULATOR
 // Exercise the A15 texture conversion paths even on the Mac's different GPU.
 [features removeObject:@"texture-compression-bc"];[features removeObject:@"texture-compression-bc-sliced-3d"];[features removeObject:@"float32-filterable"];
#endif
 graphicsContext[@"nativeFeatures"]=features;
 NSString *path=[NSBundle.mainBundle pathForResource:@"wgpu_worker" ofType:@"js"];[graphicsContext evaluateScript:[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil] withSourceURL:[NSURL fileURLWithPath:path]];
}
void PGCaptureNativeFrame(const char *name){ if(!graphicsQueue)return; NSString *value=[NSString stringWithUTF8String:name]; dispatch_async(graphicsQueue,^{@autoreleasepool {[graphicsContext[@"requestNativeFrame"] callWithArguments:@[value]];}}); }
void PGAttachMetalLayer(void *layer){metalLayer=(__bridge CAMetalLayer *)layer;}
void PGStartGraphics(void *memory,uint64_t size,uint64_t ring,uint64_t ringSize,uint64_t debug){
 engineMemory=memory;engineMemorySize=size;graphicsQueue=dispatch_queue_create("playgta5.metal",dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL,QOS_CLASS_USER_INTERACTIVE,0));
 dispatch_async(graphicsQueue,^{@autoreleasepool {
  if(!createMetalGPU()){pg_log("Native Metal adapter could not start.");return;}
  installRuntime(memory,size);
#if TARGET_OS_SIMULATOR
  for(NSString *mode in @[@"reference",@"native"]){int token;NSString *notification=[@"playgta5.pack." stringByAppendingString:mode];BOOL compiled=[mode isEqualToString:@"native"];
   notify_register_dispatch(notification.UTF8String,&token,graphicsQueue,^(int){@autoreleasepool {[graphicsContext[@"setNativePackingMode"] callWithArguments:@[@(compiled)]];nativeBridgeMs=nativeUploadMs=nativeBatchMs=0;nativeCallCount=nativeBatchCommandCount=nativePresentCount=0;memoryReportTime=NSProcessInfo.processInfo.systemUptime;}});
  }
  int frameToken;notify_register_dispatch("playgta5.frame",&frameToken,graphicsQueue,^(int){@autoreleasepool {[graphicsContext[@"requestNativeFrame"] callWithArguments:@[@"simulator-live"]];}});
  int metricsToken;notify_register_dispatch("playgta5.metrics",&metricsToken,graphicsQueue,^(int){@autoreleasepool {saveFrameSamples();}});
  int textureMetricsToken;notify_register_dispatch("playgta5.texture-metrics",&textureMetricsToken,graphicsQueue,^(int){@autoreleasepool {saveTextureSamples();}});
#endif
  [graphicsContext[@"startNativeRenderer"] callWithArguments:@[@(ring),@(ringSize),@(debug)]];
  dispatch_source_t timer=dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,0,0,graphicsQueue);
  dispatch_source_set_timer(timer,DISPATCH_TIME_NOW,5*NSEC_PER_MSEC,NSEC_PER_MSEC);
  dispatch_source_set_event_handler(timer,^{@autoreleasepool {if(instance)wgpuInstanceProcessEvents(instance);if(gpuDevice)wgpuDeviceTick(gpuDevice);
   double now=NSProcessInfo.processInfo.systemUptime;if(now-memoryReportTime>15){
    if(PGMemoryBudgetQueries()){char line[130];snprintf(line,sizeof line,"Native memory budget guard: %llu clamps in %llu queries.",PGMemoryBudgetClamps(),PGMemoryBudgetQueries());pg_log(line);}
    if(nativeOperationProfile&&nativePresentCount)reportNativeOperations();
    if(gpuDevice&&dawn::native::metal::GetMTLDevice(gpuDevice).currentAllocatedSize){char line[120];snprintf(line,sizeof line,"Native Metal resource allocations: %.0f MB",dawn::native::metal::GetMTLDevice(gpuDevice).currentAllocatedSize/1048576.0);pg_log(line);}
    if(nativePresentCount){char line[240];snprintf(line,sizeof line,"Native frame costs: bridge %.1f ms (uploads %.1f ms), batched Metal %.1f ms, %llu calls and %llu batched commands per frame",nativeBridgeMs/nativePresentCount,nativeUploadMs/nativePresentCount,nativeBatchMs/nativePresentCount,nativeCallCount/nativePresentCount,nativeBatchCommandCount/nativePresentCount);pg_log(line);nativeBridgeMs=nativeUploadMs=nativeBatchMs=0;nativeCallCount=nativeBatchCommandCount=nativePresentCount=0;}memoryReportTime=now;task_vm_info_data_t info={};mach_msg_type_number_t count=TASK_VM_INFO_COUNT;if(task_info(mach_task_self(),TASK_VM_INFO,(task_info_t)&info,&count)==KERN_SUCCESS){char line[160];snprintf(line,sizeof line,"Phone memory: footprint %.0f MB, available headroom %.0f MB, thermal state %ld",info.phys_footprint/1048576.0,os_proc_available_memory()/1048576.0,(long)NSProcessInfo.processInfo.thermalState);pg_log(line);}}
}});dispatch_resume(timer);
  // Keep the event pump alive for the lifetime of the native renderer.
  graphicsPump=timer;
 }});
}
