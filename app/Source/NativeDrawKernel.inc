// Native vertex/index setup, dynamic state and draw validation. State changes
// travel in the same ordered batch as bind groups and draws.
namespace NativeDraw {
struct Buffer {WGPUBuffer gpu;uint64_t size,encoder=0;};
static std::unordered_map<uint32_t,Buffer> buffers;
static uint32_t *counters;
struct Slot {uint32_t slot,instance;};
struct Requirement {uint32_t slot,stride,last,instance;};
struct Plan {bool foldVertex,foldInstance;std::vector<Slot> slots;std::vector<Requirement> requirements;};
struct Vertex {uint32_t id=0,stride=0;uint64_t offset=0;};
static std::array<Vertex,16> vertices;
static uint32_t indexId,indexFormat,stencil;
static uint64_t indexOffset;
static bool hasViewport,hasScissor,rasterScissor;
static std::array<double,6> viewport;
static std::array<double,4> scissor,blend;
static WGPUBuffer zeroBuffer;
struct Applied {
 WGPURenderPassEncoder pass=nullptr;
 std::array<WGPUBuffer,16> vertices{};std::array<uint64_t,16> offsets{};
 WGPUBuffer index=nullptr;uint32_t indexFormat=0;uint64_t indexOffset=0;
 bool viewportValid=false,scissorValid=false,blendValid=false,stencilValid=false;
 std::array<double,6> viewport{};std::array<uint32_t,4> scissor{};std::array<double,4> blend{};uint32_t stencil=0;
};
static Applied applied;
static void invalidate(){applied.pass=nullptr;}
static void reset(){vertices={};indexId=indexFormat=stencil=0;indexOffset=0;hasViewport=hasScissor=rasterScissor=false;viewport={};scissor={};blend={};invalidate();}
static JSValueRef registerBuffer(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *){
 if(n>=3){uint32_t id=NativeUniforms::number(c,v[0]);WGPUBuffer gpu=(WGPUBuffer)rawGPUHandle(c,v[1]);uint64_t size=JSValueToNumber(c,v[2],nullptr);if(gpu){auto old=buffers.find(id);if(old!=buffers.end())wgpuBufferRelease(old->second.gpu);wgpuBufferAddRef(gpu);buffers[id]={gpu,size,0};}}return JSValueMakeUndefined(c);
}
static JSValueRef forgetBuffer(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *){
 if(n){auto b=buffers.find(NativeUniforms::number(c,v[0]));if(b!=buffers.end()){wgpuBufferRelease(b->second.gpu);buffers.erase(b);}}return JSValueMakeUndefined(c);
}
static JSValueRef bufferUsed(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *){
 if(n<2)return JSValueMakeBoolean(c,false);auto b=buffers.find(NativeUniforms::number(c,v[0]));uint64_t encoder=JSValueToNumber(c,v[1],nullptr);return JSValueMakeBoolean(c,b!=buffers.end()&&b->second.encoder==encoder);
}
static JSValueRef makePlan(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(!n)return JSValueMakeNull(c);JSObjectRef a=JSValueToObject(c,v[0],error);size_t bytes=JSObjectGetTypedArrayByteLength(c,a,error);uint32_t *p=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,a,error);if(bytes<16||bytes%4||bytes>1024||!p)return JSValueMakeNull(c);
 size_t words=bytes/4,at=3;uint32_t slots=p[2];if(slots>16||at+slots*2+1>words)return JSValueMakeNull(c);Plan *plan=new Plan{p[0]!=0,p[1]!=0,{},{}};
 for(uint32_t i=0;i<slots;i++){if(p[at]>=16){delete plan;return JSValueMakeNull(c);}plan->slots.push_back({p[at],p[at+1]});at+=2;}
 uint32_t req=p[at++];if(req>16||at+req*4!=words){delete plan;return JSValueMakeNull(c);}
 for(uint32_t i=0;i<req;i++){if(p[at]>=16){delete plan;return JSValueMakeNull(c);}plan->requirements.push_back({p[at],p[at+1],p[at+2],p[at+3]});at+=4;}
 return wrapGPU(graphicsContext,@"drawPlan",plan,[](void *p){delete (Plan *)p;}).JSValueRef;
}
static Buffer *buffer(uint32_t id){auto b=buffers.find(id);return b==buffers.end()?nullptr:&b->second;}
static WGPUBuffer zeros(){if(!zeroBuffer){auto d=WGPU_BUFFER_DESCRIPTOR_INIT;d.size=4096;d.usage=WGPUBufferUsage_Vertex;zeroBuffer=wgpuDeviceCreateBuffer(gpuDevice,&d);}return zeroBuffer;}
static void draw(WGPURenderPassEncoder pass,Plan *plan,const double *p){
 if(!pass||!plan){counters[1]++;return;}if(applied.pass!=pass){applied=Applied{};applied.pass=pass;}
 uint32_t kind=p[0],count=p[1],instances=p[2],start=p[3],firstInstance=p[5];int32_t base=p[4];uint64_t encoder=p[6];uint32_t width=p[7],height=p[8];
 bool indexed=kind==4||(kind<4&&(kind&1));uint32_t vertexFirst=indexed||plan->foldVertex?0:start,instanceFirst=plan->foldInstance?0:firstInstance;
 std::array<int64_t,16> available{};
 for(const auto &slot:plan->slots){const auto &vb=vertices[slot.slot];Buffer *b=buffer(vb.id);WGPUBuffer gpu=b?b->gpu:zeros();int64_t shift=slot.instance==1?(plan->foldInstance?(int64_t)firstInstance*vb.stride:0):(plan->foldVertex?(int64_t)(indexed?base:start)*vb.stride:0);uint64_t offset=b?std::max((int64_t)0,(int64_t)vb.offset+shift):0;
  if(applied.vertices[slot.slot]!=gpu||applied.offsets[slot.slot]!=offset){wgpuRenderPassEncoderSetVertexBuffer(pass,slot.slot,gpu,offset,WGPU_WHOLE_SIZE);applied.vertices[slot.slot]=gpu;applied.offsets[slot.slot]=offset;}
  available[slot.slot]=b?(int64_t)b->size-(int64_t)offset:4096;if(b)b->encoder=encoder;
 }
 int64_t indexMaximum=INT64_MAX;
 if(indexed){Buffer *b=buffer(indexId);if(!b){counters[1]++;return;}uint32_t format=indexFormat==57?2:4;indexMaximum=((int64_t)b->size-(int64_t)indexOffset)/format-start;
  if(applied.index!=b->gpu||applied.indexFormat!=format||applied.indexOffset!=indexOffset){wgpuRenderPassEncoderSetIndexBuffer(pass,b->gpu,format==2?WGPUIndexFormat_Uint16:WGPUIndexFormat_Uint32,indexOffset,WGPU_WHOLE_SIZE);applied.index=b->gpu;applied.indexFormat=format;applied.indexOffset=indexOffset;}b->encoder=encoder;
 }
 if(hasViewport&&(!applied.viewportValid||applied.viewport!=viewport)){wgpuRenderPassEncoderSetViewport(pass,viewport[0],viewport[1],std::max(1.0,viewport[2]),std::max(1.0,viewport[3]),viewport[4],viewport[5]);applied.viewport=viewport;applied.viewportValid=true;}
 std::array<uint32_t,4> clip{0,0,width,height};if(rasterScissor&&hasScissor){uint32_t x=std::min((double)width,scissor[0]),y=std::min((double)height,scissor[1]),right=std::min((double)width,std::max((double)x,scissor[2])),bottom=std::min((double)height,std::max((double)y,scissor[3]));clip={x,y,right-x,bottom-y};}
 if(!applied.scissorValid||applied.scissor!=clip){wgpuRenderPassEncoderSetScissorRect(pass,clip[0],clip[1],clip[2],clip[3]);applied.scissor=clip;applied.scissorValid=true;}
 if(!applied.blendValid||applied.blend!=blend){WGPUColor color{blend[0],blend[1],blend[2],blend[3]};wgpuRenderPassEncoderSetBlendConstant(pass,&color);applied.blend=blend;applied.blendValid=true;}
 if(!applied.stencilValid||applied.stencil!=stencil){wgpuRenderPassEncoderSetStencilReference(pass,stencil);applied.stencil=stencil;applied.stencilValid=true;}
 if(kind>=4){Buffer *args=buffer(count);if(!args){counters[1]++;return;}args->encoder=encoder;counters[2]++;if(kind==4)wgpuRenderPassEncoderDrawIndexedIndirect(pass,args->gpu,instances);else wgpuRenderPassEncoderDrawIndirect(pass,args->gpu,instances);return;}
 int64_t drawCount=count,drawInstances=instances;
 for(const auto &r:plan->requirements){int64_t max=available[r.slot]<(int64_t)r.last?0:r.stride?(available[r.slot]-r.last)/r.stride+1:INT64_MAX;
  if(r.instance)drawInstances=std::min(drawInstances,max-instanceFirst);else if(!indexed)drawCount=std::min(drawCount,max-vertexFirst);
 }
 if(indexed)drawCount=std::min(drawCount,indexMaximum);if(drawCount!=count||drawInstances!=instances)counters[0]++;if(drawCount<=0||drawInstances<=0){counters[1]++;return;}counters[2]++;
 if(indexed)wgpuRenderPassEncoderDrawIndexed(pass,drawCount,drawInstances,start,plan->foldVertex?0:base,instanceFirst);else wgpuRenderPassEncoderDraw(pass,drawCount,drawInstances,vertexFirst,instanceFirst);
}
static void install(JSContext *context){JSContextRef c=context.JSGlobalContextRef;JSObjectRef array=JSObjectMakeTypedArray(c,kJSTypedArrayTypeUint32Array,3,nullptr);counters=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,array,nullptr);context[@"nativeDrawCounters"]=[JSValue valueWithJSValueRef:array inContext:context];NativeUniforms::expose(c,"nativeRegisterDrawBuffer",registerBuffer);NativeUniforms::expose(c,"nativeForgetDrawBuffer",forgetBuffer);NativeUniforms::expose(c,"nativeDrawBufferUsed",bufferUsed);NativeUniforms::expose(c,"nativeMakeDrawPlan",makePlan);}
}
