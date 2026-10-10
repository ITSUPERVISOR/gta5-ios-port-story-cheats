// A warm draw resolves its pipeline and bindings in one C API transition.
// Snapshots replay in the existing ordered GPU batch, including vertex state.
#include <unordered_set>
namespace NativePreparedDraw {
struct Record {
 JSValueRef owner=nullptr;
 WGPURenderPipeline pipeline=nullptr;
 NativeDraw::Plan *plan=nullptr;
 std::array<JSValueRef,2> caches{},proxies{};
 std::array<NativeUniforms::BindingCache *,2> nativeCaches{};
 std::array<NativeUniforms::ProxyPlan *,2> nativeProxies{};
 std::array<bool,2> needed{};
 uint32_t generation=0;
 uint64_t pipelineGeneration=0;
};
struct Snapshot {
 WGPURenderPipeline pipeline=nullptr;
 NativeDraw::Plan *plan=nullptr;
 std::array<WGPUBindGroup,2> groups{};
 std::array<uint32_t,2> counts{},offsets{};
 std::array<double,9> draw{};
};
static std::unordered_map<NativePipeline::Key,Record,NativePipeline::Hash> records;
static std::unordered_map<uint32_t,NativePipeline::Layout> layouts;
static std::vector<Snapshot> snapshots;
static std::unordered_set<JSValueRef> roots;
static std::unordered_map<uint32_t,uint64_t> resourceUsage;
static bool nativeResourceMarkers;
static bool directResolution=true;
static bool cacheUsageStamps=true;
static bool preparedProxies=true;
static JSValueRef resourceUsed(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *){
 if(n<2)return JSValueMakeBoolean(c,false);
 auto found=resourceUsage.find(NativeUniforms::number(c,v[0]));
 return JSValueMakeBoolean(c,found!=resourceUsage.end()&&found->second==(uint64_t)JSValueToNumber(c,v[1],nullptr));
}
static JSValueRef forgetResource(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *){
 if(n)resourceUsage.erase(NativeUniforms::number(c,v[0]));return JSValueMakeUndefined(c);
}
struct ReplayState {
 WGPURenderPassEncoder pass=nullptr;
 WGPURenderPipeline pipeline=nullptr;
 std::array<WGPUBindGroup,2> groups{};
 std::array<uint32_t,2> counts{},offsets{};
};
static ReplayState replayState;
static void invalidateReplay(){replayState=ReplayState{};}
static JSStringRef field(const char *name){
 static std::unordered_map<std::string,JSStringRef> fields;
 auto found=fields.find(name);if(found!=fields.end())return found->second;
 JSStringRef value=JSStringCreateWithUTF8CString(name);fields.emplace(name,value);return value;
}
static JSValueRef property(JSContextRef c,JSObjectRef object,const char *name,JSValueRef *error){return JSObjectGetProperty(c,object,field(name),error);}
static JSValueRef registerPipeline(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(n<11||!JSValueIsObjectOfClass(c,v[0],NativePipeline::layoutClass()))return JSValueMakeUndefined(c);
 NativePipeline::Layout *layout=(NativePipeline::Layout *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));
 if(!layout)return JSValueMakeUndefined(c);
 uint32_t tail=NativeUniforms::number(c,v[3]),generation=NativeUniforms::number(c,v[4]);
 auto identity=NativePipeline::key(layout,tail);
 auto previous=records.find(identity);
 if(previous!=records.end()&&previous->second.generation==generation&&previous->second.pipelineGeneration==NativePipeline::generation)return JSValueMakeUndefined(c);
 if(records.size()>=65536){records.clear();layouts.clear();}
 Record record;record.owner=v[1];record.pipeline=(WGPURenderPipeline)rawGPUHandle(c,v[1]);record.plan=(NativeDraw::Plan *)rawGPUHandle(c,v[2]);record.generation=generation;
 record.pipelineGeneration=NativePipeline::generation;
 for(uint32_t stage=0;stage<2;stage++){
  record.caches[stage]=JSValueIsObject(c,v[5+stage])?v[5+stage]:nullptr;
  record.proxies[stage]=JSValueIsObject(c,v[7+stage])?v[7+stage]:nullptr;
  record.needed[stage]=JSValueToBoolean(c,v[9+stage]);
  if(record.caches[stage]&&JSValueIsObjectOfClass(c,record.caches[stage],NativeUniforms::cacheClass()))record.nativeCaches[stage]=(NativeUniforms::BindingCache *)JSObjectGetPrivate(JSValueToObject(c,record.caches[stage],error));
  if(record.proxies[stage]&&JSValueIsObjectOfClass(c,record.proxies[stage],NativeUniforms::proxyClass()))record.nativeProxies[stage]=(NativeUniforms::ProxyPlan *)JSObjectGetPrivate(JSValueToObject(c,record.proxies[stage],error));
 }
 if(record.pipeline&&record.plan){layouts[NativePipeline::state[2]]=*layout;records[identity]=record;}
 return JSValueMakeUndefined(c);
}
enum class Miss {Arguments,Layout,Record,ResourceGeneration,PipelineGeneration,PipelineOwner,Proxy,Bindings,DrawRange,Exception,Count};
static std::array<uint64_t,(size_t)Miss::Count> misses{};
static int32_t failed(Miss reason,int code=-1){if(nativeOperationProfile)misses[(size_t)reason]++;return code;}
static JSValueRef miss(JSContextRef c,Miss reason,int code=-1){return JSValueMakeNumber(c,failed(reason,code));}
static void reportMisses(){
 const char *names[]={"arguments","layout","record","resource generation","pipeline generation","pipeline owner","texture proxy","bindings","draw range","exception"};
 NSMutableString *line=[NSMutableString stringWithString:@"Native prepared draw fallback reasons:"];
 for(size_t i=0;i<misses.size();i++)if(misses[i])[line appendFormat:@" %s %llu;",names[i],misses[i]];
 pg_log(line.UTF8String);misses.fill(0);NativeUniforms::reportUploads();NativeUniforms::reportPackets();
}
struct DrawContext {
 uint32_t tail=0,epoch=0,slot=0,slots=0,resourceGeneration=0;
 uint64_t frame=0;
 double encoder=0,width=0,height=0;
 JSValueRef encoderValue=nullptr;
};
// Numeric entry point for a compiled command span. The public scripting call
// still uses this same resolver and snapshot ownership/ordering checks.
static int32_t prepareNative(JSContextRef c,double wordIndex,const DrawContext &context,JSValueRef *error){
 if(snapshots.size()>=65536)return failed(Miss::Arguments);
 JSValueRef encoderValue=context.encoderValue?context.encoderValue:JSValueMakeNumber(c,context.encoder);
 auto layout=layouts.find(NativePipeline::state[2]);if(layout==layouts.end())return failed(Miss::Layout);
 auto identity=NativePipeline::key(&layout->second,context.tail);
 auto found=records.find(identity);if(found==records.end())return failed(Miss::Record);
 Record &record=found->second;
 // JS invalidates programs on resource destruction and cache trimming. Check
 // the generation before touching pointers whose capsules may have been freed.
 if(record.generation!=context.resourceGeneration)return failed(Miss::ResourceGeneration);
 if(record.pipelineGeneration!=NativePipeline::generation)return failed(Miss::PipelineGeneration);
 auto pipeline=NativePipeline::values.find(identity);
 if(pipeline==NativePipeline::values.end()||pipeline->second!=record.owner)return failed(Miss::PipelineOwner);
 uint32_t epoch=context.epoch,slot=context.slot,slots=context.slots;
 uint64_t frame=context.frame;
 std::array<uint32_t,2> proxyMasks{};
 for(uint32_t stage=0;stage<2;stage++)if(record.proxies[stage]){
  uint32_t mask;
  if(directResolution){
   if(!record.nativeProxies[stage])return failed(Miss::Bindings);
   mask=NativeUniforms::proxyMaskNative(record.nativeProxies[stage],stage,epoch);
  }else{
   JSValueRef args[]={record.proxies[stage],JSValueMakeNumber(c,stage),JSValueMakeNumber(c,context.epoch)};
   mask=JSValueToNumber(c,NativeUniforms::proxyMask(c,nullptr,nullptr,3,args,error),error);
  }
  proxyMasks[stage]=mask;if(mask&&!preparedProxies)return failed(Miss::Proxy,-2);
 }
 std::array<NativeUniforms::BindingKey *,2> keys{};
 for(uint32_t stage=0;stage<2;stage++)if(record.needed[stage]){
  auto *cache=record.nativeCaches[stage];if(!cache)return failed(Miss::Bindings);
  NativeUniforms::BindingKey *key;
  if(directResolution)key=NativeUniforms::resolveCacheNative(c,cache,stage,epoch,frame,slot,slots,false,error);
  else{
   JSValueRef args[]={record.caches[stage],JSValueMakeNumber(c,stage),JSValueMakeNumber(c,context.epoch),JSValueMakeNumber(c,context.frame),JSValueMakeNumber(c,context.slot),JSValueMakeNumber(c,context.slots)};
   JSValueRef entry=NativeUniforms::resolveBindings(c,nullptr,nullptr,6,args,error);
   if(!JSValueIsObject(c,entry))return failed(Miss::Bindings);
   uint32_t at=cache->last[stage];if(at>=cache->keys[stage].size())return failed(Miss::Bindings);key=&cache->keys[stage][at];
  }
  if(!key||!key->group||key->dynamicCount>1)return failed(Miss::Bindings);keys[stage]=key;
 }
 for(uint32_t stage=0;stage<2;stage++)if(proxyMasks[stage]&&(!keys[stage]||!NativeUniforms::proxiesReady(c,*keys[stage],proxyMasks[stage],error)))return failed(Miss::Proxy,-2);
 if(engineMemorySize<24||!std::isfinite(wordIndex)||wordIndex<0||floor(wordIndex)!=wordIndex||wordIndex>(double)(engineMemorySize-24)/4)return failed(Miss::DrawRange);
 const uint32_t *words=(const uint32_t *)((const uint8_t *)engineMemory+(uint64_t)wordIndex*4);
 Snapshot snapshot;snapshot.pipeline=record.pipeline;snapshot.plan=record.plan;
 snapshot.draw={(double)words[0],(double)words[1],(double)words[2],(double)words[3],(double)(int32_t)words[4],(double)words[5],context.encoder,context.width,context.height};
 for(uint32_t stage=0;stage<2;stage++)if(keys[stage]){
  auto &key=*keys[stage];snapshot.groups[stage]=key.group;snapshot.counts[stage]=key.dynamicCount;
  if(key.dynamicCount)snapshot.offsets[stage]=directResolution?key.nativeDynamicOffset:NativeUniforms::number(c,JSObjectGetPropertyAtIndex(c,key.dynamic,0,error));
  // JS resource stamps only move forward when flush() advances encEpoch.
  // Reusing this immutable bind-group key in the same encoder needs no new
  // property writes. Dirty proxy flags still run for every draw because a
  // proxy refresh can clear them between draws in the same encoder.
  uint64_t encoderEpoch=(uint64_t)snapshot.draw[6];
  NativeUniforms::stampResources(key,encoderEpoch,cacheUsageStamps,[&](size_t i){
   uint32_t id=key.usedIds[i];
   if(nativeResourceMarkers&&id)resourceUsage[id]=encoderEpoch;
   else JSObjectSetProperty(c,key.used[i],field("enc"),encoderValue,kJSPropertyAttributeNone,error);
   return !error||!*error;
  });
  for(JSObjectRef resource:key.dirty)for(const char *name:{"proxy_fdepth","proxy_filter"}){
   JSValueRef proxy=property(c,resource,name,error);
   if(JSValueIsObject(c,proxy))JSObjectSetProperty(c,JSValueToObject(c,proxy,error),field("dirty"),JSValueMakeBoolean(c,true),kJSPropertyAttributeNone,error);
  }
 }
 if(error&&*error)return failed(Miss::Exception);
 if(roots.insert(record.owner).second)JSValueProtect(c,record.owner);
 wgpuRenderPipelineAddRef(snapshot.pipeline);
 for(WGPUBindGroup group:snapshot.groups)if(group)wgpuBindGroupAddRef(group);
 uint32_t index=snapshots.size();snapshots.push_back(snapshot);return index;
}
static JSValueRef prepare(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 NativeCallClock timing(false,"native.prepareDraw");
 if(n<10)return miss(c,Miss::Arguments);
 DrawContext context;
 context.tail=NativeUniforms::number(c,v[1]);context.epoch=NativeUniforms::number(c,v[2]);
 context.frame=JSValueToNumber(c,v[3],nullptr);context.slot=NativeUniforms::number(c,v[4]);context.slots=NativeUniforms::number(c,v[5]);
 context.encoder=JSValueToNumber(c,v[6],error);context.encoderValue=v[6];context.resourceGeneration=NativeUniforms::number(c,v[7]);
 context.width=JSValueToNumber(c,v[8],error);context.height=JSValueToNumber(c,v[9],error);
 return JSValueMakeNumber(c,prepareNative(c,JSValueToNumber(c,v[0],error),context,error));
}
static void replay(uint32_t index,WGPURenderPassEncoder pass){
 if(index>=snapshots.size()||!pass){pg_log("Invalid native prepared draw snapshot.");return;}
 const Snapshot &snapshot=snapshots[index];
 if(replayState.pass!=pass){invalidateReplay();replayState.pass=pass;}
 if(replayState.pipeline!=snapshot.pipeline){wgpuRenderPassEncoderSetPipeline(pass,snapshot.pipeline);replayState.pipeline=snapshot.pipeline;}
 for(uint32_t stage=0;stage<2;stage++)if(snapshot.groups[stage]&&(replayState.groups[stage]!=snapshot.groups[stage]||replayState.counts[stage]!=snapshot.counts[stage]||replayState.offsets[stage]!=snapshot.offsets[stage])){
  wgpuRenderPassEncoderSetBindGroup(pass,stage,snapshot.groups[stage],snapshot.counts[stage],snapshot.counts[stage]?&snapshot.offsets[stage]:nullptr);
  replayState.groups[stage]=snapshot.groups[stage];replayState.counts[stage]=snapshot.counts[stage];replayState.offsets[stage]=snapshot.offsets[stage];
 }
 NativeDraw::draw(pass,snapshot.plan,snapshot.draw.data());
}
static void clearSnapshots(JSContextRef c){
 for(const Snapshot &snapshot:snapshots){wgpuRenderPipelineRelease(snapshot.pipeline);for(WGPUBindGroup group:snapshot.groups)if(group)wgpuBindGroupRelease(group);}
 snapshots.clear();for(JSValueRef owner:roots)JSValueUnprotect(c,owner);roots.clear();invalidateReplay();
}
static void install(JSContext *context){
 context[@"nativeFixedStatePacking"]=@(![NSProcessInfo.processInfo.arguments containsObject:@"--reference-state-packing"]);
 nativeDetailProfileReport=reportMisses;
 preparedProxies=![NSProcessInfo.processInfo.arguments containsObject:@"--reference-proxy-snapshots"];
 context[@"nativePreparedProxyDraws"]=@(preparedProxies);
 cacheUsageStamps=[NSProcessInfo.processInfo.arguments containsObject:@"--native-usage-stamps"]&&![NSProcessInfo.processInfo.arguments containsObject:@"--reference-usage-stamps"];
 context[@"nativeCachedUsageStamps"]=@(cacheUsageStamps);
 directResolution=![NSProcessInfo.processInfo.arguments containsObject:@"--reference-draw-resolver"];
 context[@"nativeDirectDrawResolver"]=@(directResolution);
 nativeResourceMarkers=[NSProcessInfo.processInfo.arguments containsObject:@"--native-resource-usage"]&&![NSProcessInfo.processInfo.arguments containsObject:@"--reference-resource-usage"];
 context[@"nativeResourceMarkers"]=@(nativeResourceMarkers);
 context[@"nativePreparedDraws"]=@(![NSProcessInfo.processInfo.arguments containsObject:@"--reference-prepared-draws"]);
 NativeUniforms::expose(context.JSGlobalContextRef,"nativeResourceUsed",resourceUsed);
 NativeUniforms::expose(context.JSGlobalContextRef,"nativeForgetResourceUsage",forgetResource);
 NativeUniforms::expose(context.JSGlobalContextRef,"nativeRegisterPreparedPipeline",registerPipeline);
 NativeUniforms::expose(context.JSGlobalContextRef,"nativePrepareDraw",prepare);
}
}
