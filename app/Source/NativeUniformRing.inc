// Allocate, copy and upload compiled shader constants without per-draw JS ring work.
// The renderer's existing frame-slot fence protects both native and reference rings.
static constexpr uint32_t RingBytes=8u<<20,UniformWindow=65536;
struct RingStage {std::vector<uint8_t> bytes;uint32_t low=UINT32_MAX,high=0;};
struct RingChunk {WGPUBuffer gpu=nullptr;std::array<RingStage,8> stages;};
static std::vector<RingChunk> ringChunks;
static uint64_t ringFrame=UINT64_MAX;
static uint32_t ringChunkIndex=0,ringOffset=0,ringSlot=0,ringSlots=0;
static uint32_t *ringAllocation;
static JSObjectRef ringBufferObjects;
static WGPUQueue ringQueue;
static int32_t allocatePackNative(JSContextRef c,Pack *pack,uint32_t stage,uint64_t frame,uint32_t slot,uint32_t slots,uint32_t size,JSValueRef *error){
 if(!pack||stage>=3||!slots||slots>8||slot>=slots||!size||size>UniformWindow||(ringSlots&&ringSlots!=slots))return -1;
 for(const auto &m:pack->members)if(m.offset>size||m.size>size-m.offset)return -1;
 if(ringFrame!=frame){ringFrame=frame;ringChunkIndex=ringOffset=0;ringSlot=slot;ringSlots=slots;}
 if(ringSlot!=slot)return -1;
 Key key=currentKey(pack,stage);
 for(uint32_t i=0;i<6;i++){const Entry &e=pack->entries[stage][i];if(e.valid&&e.frame==frame&&equal(e.key,key,pack->members.size())){ringAllocation[0]=e.chunk;ringAllocation[1]=e.offset;return i|256;}}
 uint32_t aligned=(size+255)&~255u;
 if(ringOffset+aligned+UniformWindow>RingBytes){ringChunkIndex++;ringOffset=0;}
 if(ringChunkIndex>=ringChunks.size()){
  auto descriptor=WGPU_BUFFER_DESCRIPTOR_INIT;descriptor.size=(uint64_t)RingBytes*slots;descriptor.usage=WGPUBufferUsage_Uniform|WGPUBufferUsage_CopyDst;
  WGPUBuffer buffer=wgpuDeviceCreateBuffer(gpuDevice,&descriptor);if(!buffer)return -1;
  RingChunk chunk;chunk.gpu=buffer;ringChunks.push_back(std::move(chunk));
  JSValue *wrapped=wrapGPU(graphicsContext,@"buffer",buffer,[](void *p){wgpuBufferRelease((WGPUBuffer)p);});
  JSObjectSetPropertyAtIndex(c,ringBufferObjects,ringChunkIndex,wrapped.JSValueRef,error);
 }
 RingStage &destination=ringChunks[ringChunkIndex].stages[slot];uint32_t offset=ringOffset,end=offset+size;
 if(destination.bytes.size()<end)destination.bytes.resize(std::min((size_t)RingBytes,std::max((size_t)end,std::max((size_t)262144,destination.bytes.size()*2))));
 for(size_t i=0;i<pack->members.size();i++){
  const Member &m=pack->members[i];uint8_t *out=destination.bytes.data()+offset+m.offset;size_t copied=0;
  auto source=shadows.find(key.ids[i]);if(source!=shadows.end()){
   size_t first=(size_t)key.firsts[i]*16;if(first<source->second.size){copied=std::min((size_t)m.size,source->second.size-first);memcpy(out,source->second.bytes+first,copied);}
  }
  if(copied<m.size)memset(out+copied,0,m.size-copied);
 }
 ringOffset+=aligned;destination.low=std::min(destination.low,offset);destination.high=std::max(destination.high,end);
 uint32_t index=pack->next[stage]++%6;Entry &entry=pack->entries[stage][index];entry.valid=true;entry.frame=frame;entry.key=key;entry.chunk=ringChunkIndex;entry.offset=offset;
 ringAllocation[0]=ringChunkIndex;ringAllocation[1]=offset;return index;
}
static JSValueRef allocatePack(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(n<6)return JSValueMakeNumber(c,-1);
 Pack *pack=(Pack *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));
 return JSValueMakeNumber(c,allocatePackNative(c,pack,number(c,v[1]),JSValueToNumber(c,v[2],nullptr),number(c,v[3]),number(c,v[4]),number(c,v[5]),error));
}
// Prepared snapshots own the offset value. Synchronize the JS array only when
// its caller needs that array, including hits after an earlier native resolution.
static BindingKey *resolveCacheNative(JSContextRef c,BindingCache *cache,uint32_t stage,uint32_t epoch,uint64_t frame,uint32_t slot,uint32_t slots,bool syncJS,JSValueRef *error){
 if(!cache||stage>=3)return nullptr;uint32_t hit=findCache(c,cache,stage,epoch);if(!hit)return nullptr;
 BindingKey &key=cache->keys[stage][(hit&255)-1];if(!key.entry)return nullptr;
 if(key.pack&&(!(hit&256)||key.frame!=frame)){
  int32_t allocation=allocatePackNative(c,key.pack,stage,frame,slot,slots,key.packSize,error);if(allocation<0)return nullptr;
  const Entry &entry=key.pack->entries[stage][allocation&255];
  if(0x40000000u+entry.chunk!=key.chunkId)return nullptr;
  key.nativeDynamicOffset=slot*RingBytes+entry.offset;key.dynamicDirty=true;key.frame=frame;
  for(size_t i=0;i<key.ids.size();i++){uint32_t kind=cache->sources[i*2];if(kind<2){key.ids[i]=bound(stage,kind,cache->sources[i*2+1]);if(kind==0)key.versions[i]=version(key.ids[i]);}}
 }
 if(syncJS&&key.dynamicDirty){
  JSObjectSetPropertyAtIndex(c,key.dynamic,0,JSValueMakeNumber(c,key.nativeDynamicOffset),error);
  if(error&&*error)return nullptr;key.dynamicDirty=false;
 }
 return &key;
}
static JSValueRef resolveBindings(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(n<6)return JSValueMakeNull(c);
 BindingCache *cache=(BindingCache *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));
 BindingKey *key=resolveCacheNative(c,cache,number(c,v[1]),number(c,v[2]),JSValueToNumber(c,v[3],nullptr),number(c,v[4]),number(c,v[5]),true,error);
 return key?key->entry:JSValueMakeNull(c);
}
static JSValueRef flushRing(JSContextRef c,JSObjectRef,JSObjectRef,size_t,const JSValueRef[],JSValueRef *){
 if(ringChunks.empty())return JSValueMakeUndefined(c);
 NativeCallClock timing(true,"native.uniformFlush");
 if(!ringChunks.empty()&&!ringQueue)ringQueue=wgpuDeviceGetQueue(gpuDevice);
 for(auto &chunk:ringChunks)for(uint32_t slot=0;slot<ringSlots;slot++){
  RingStage &stage=chunk.stages[slot];if(stage.low==UINT32_MAX)continue;
  uint32_t high=(stage.high+3)&~3u;
  if(stage.bytes.size()<high)stage.bytes.resize(high);
  wgpuQueueWriteBuffer(ringQueue,chunk.gpu,(uint64_t)slot*RingBytes+stage.low,stage.bytes.data()+stage.low,high-stage.low);
  stage.low=UINT32_MAX;stage.high=0;
 }
 return JSValueMakeUndefined(c);
}
static void installRing(JSContext *context){
 JSContextRef c=context.JSGlobalContextRef;JSObjectRef allocation=JSObjectMakeTypedArray(c,kJSTypedArrayTypeUint32Array,2,nullptr);
 ringAllocation=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,allocation,nullptr);context[@"nativeUniformAllocation"]=[JSValue valueWithJSValueRef:allocation inContext:context];
 ringBufferObjects=JSObjectMakeArray(c,0,nullptr,nullptr);context[@"nativeUniformBuffers"]=[JSValue valueWithJSValueRef:ringBufferObjects inContext:context];
 expose(c,"nativeAllocateUniformPack",allocatePack);expose(c,"nativeFlushUniformRing",flushRing);expose(c,"nativeResolveBindingCache",resolveBindings);
}
