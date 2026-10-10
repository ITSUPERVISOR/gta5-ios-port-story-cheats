// Constant-buffer cache and copies execute as compiled C++, without per-member JS work.
#include <unordered_map>
#include <array>
namespace NativeUniforms {
struct Shadow {uint8_t *bytes;size_t size;uint32_t version;};
static std::unordered_map<uint32_t,Shadow> shadows;
static bool cacheUploadContents=false;
struct UniformUploadStats {uint64_t calls=0,bytes=0,unchangedCalls=0,unchangedBytes=0;};
static UniformUploadStats uploadStats;
static void reportUploads(){
 if(uploadStats.calls){NSString *line=[NSString stringWithFormat:@"Native constant uploads: %llu calls, %llu bytes, %llu unchanged calls, %llu unchanged bytes; content cache %@.",uploadStats.calls,uploadStats.bytes,uploadStats.unchangedCalls,uploadStats.unchangedBytes,cacheUploadContents?@"on":@"off"];pg_log(line.UTF8String);}
 uploadStats={};
}
static uint32_t *state;
static uint32_t *bindings;
struct Member {uint32_t slot,size,offset;};
struct Key {std::array<uint32_t,16> ids{},versions{},firsts{};};
struct Entry {bool valid=false;uint64_t frame=0;Key key;uint32_t chunk=0,offset=0;};
struct Pack {std::vector<Member> members;std::array<std::array<Entry,6>,3> entries{};std::array<uint32_t,3> next{};};
static JSClassRef packClass() {
 static JSClassRef value;static dispatch_once_t once;
 dispatch_once(&once,^{JSClassDefinition d=kJSClassDefinitionEmpty;d.className="CompiledUniformPack";d.finalize=[](JSObjectRef object){delete (Pack *)JSObjectGetPrivate(object);};value=JSClassCreate(&d);});return value;
}
static uint32_t number(JSContextRef c,JSValueRef value){return (uint32_t)JSValueToNumber(c,value,nullptr);}
static JSValueRef makeBuffer(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error) {
 if(n<2)return JSValueMakeNull(c);uint32_t id=number(c,v[0]);size_t size=number(c,v[1]);
 uint8_t *bytes=(uint8_t *)calloc(std::max((size_t)1,size),1);if(!bytes)return JSValueMakeNull(c);
 JSObjectRef buffer=JSObjectMakeArrayBufferWithBytesNoCopy(c,bytes,size,[](void *p,void *){free(p);},nullptr,error);
 if(!buffer){free(bytes);return JSValueMakeNull(c);}shadows[id]={bytes,size,0};return buffer;
}
static bool uploadNative(uint32_t id,size_t offset,size_t source,size_t size) {
 auto found=shadows.find(id);if(found==shadows.end())return false;Shadow &s=found->second;
 if(offset<=s.size&&size<=s.size-offset&&source<=engineMemorySize&&size<=engineMemorySize-source){
  const uint8_t *input=(const uint8_t *)engineMemory+source;
  bool unchanged=(cacheUploadContents||nativeOperationProfile)&&(!size||!memcmp(s.bytes+offset,input,size));
  if(nativeOperationProfile){uploadStats.calls++;uploadStats.bytes+=size;if(unchanged){uploadStats.unchangedCalls++;uploadStats.unchangedBytes+=size;}}
  if(!cacheUploadContents||!unchanged){memcpy(s.bytes+offset,input,size);s.version++;}
  return true;
 }
 pg_log("Invalid compiled constant-buffer upload.");return false;
}
static JSValueRef upload(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *) {
 NativeOperationClock timing("native.uploadUniform");
 if(n>=4)uploadNative(number(c,v[0]),number(c,v[1]),(size_t)JSValueToNumber(c,v[2],nullptr),number(c,v[3]));
 return JSValueMakeUndefined(c);
}
static JSValueRef forget(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *){if(n)shadows.erase(number(c,v[0]));return JSValueMakeUndefined(c);}
static JSValueRef makePack(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error) {
 if(!n)return JSValueMakeNull(c);JSObjectRef array=JSValueToObject(c,v[0],error);size_t size=JSObjectGetTypedArrayByteLength(c,array,error);
 uint32_t *words=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,array,error);if(!words||size%12||size>16*12)return JSValueMakeNull(c);
 Pack *p=new Pack;for(size_t i=0;i<size/4;i+=3){if(words[i]>=16){delete p;return JSValueMakeNull(c);}p->members.push_back({words[i],words[i+1],words[i+2]});}
 return JSObjectMake(c,packClass(),p);
}
static Key currentKey(Pack *p,uint32_t stage) {
 Key key;for(size_t i=0;i<p->members.size();i++){uint32_t at=(stage*16+p->members[i].slot)*3;key.ids[i]=state[at];key.firsts[i]=state[at+1];auto s=shadows.find(key.ids[i]);key.versions[i]=s==shadows.end()?UINT32_MAX:s->second.version;}return key;
}
static bool equal(const Key &a,const Key &b,size_t n){for(size_t i=0;i<n;i++)if(a.ids[i]!=b.ids[i]||a.firsts[i]!=b.firsts[i]||a.versions[i]!=b.versions[i])return false;return true;}
static JSValueRef lookup(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error) {
 if(n<3)return JSValueMakeNumber(c,-1);Pack *p=(Pack *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));uint32_t stage=number(c,v[1]);if(!p||stage>=3)return JSValueMakeNumber(c,-1);
 uint64_t frame=(uint64_t)JSValueToNumber(c,v[2],nullptr);Key key=currentKey(p,stage);
 for(uint32_t i=0;i<6;i++){const Entry &e=p->entries[stage][i];if(e.valid&&e.frame==frame&&equal(e.key,key,p->members.size()))return JSValueMakeNumber(c,i);}return JSValueMakeNumber(c,-1);
}
static JSValueRef copy(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error) {
 if(n<5)return JSValueMakeNumber(c,-1);Pack *p=(Pack *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));uint32_t stage=number(c,v[1]);if(!p||stage>=3)return JSValueMakeNumber(c,-1);
 uint64_t frame=(uint64_t)JSValueToNumber(c,v[2],nullptr);JSObjectRef array=JSValueToObject(c,v[3],error);size_t length=JSObjectGetTypedArrayByteLength(c,array,error),offset=number(c,v[4]);
 uint8_t *dst=(uint8_t *)JSObjectGetTypedArrayBytesPtr(c,array,error);if(!dst)return JSValueMakeNumber(c,-1);
 Key key=currentKey(p,stage);
 for(size_t i=0;i<p->members.size();i++){
  const Member &m=p->members[i];size_t target=offset+m.offset;if(target>length||m.size>length-target)return JSValueMakeNumber(c,-1);
  size_t copied=0;auto source=shadows.find(key.ids[i]);if(source!=shadows.end()){
   size_t first=(size_t)key.firsts[i]*16;if(first<source->second.size){copied=std::min((size_t)m.size,source->second.size-first);memcpy(dst+target,source->second.bytes+first,copied);}
  }
  if(copied<m.size)memset(dst+target+copied,0,m.size-copied);
 }
 uint32_t index=p->next[stage]++%6;Entry &entry=p->entries[stage][index];entry.valid=true;entry.frame=frame;entry.key=key;return JSValueMakeNumber(c,index);
}
static void expose(JSContextRef c,const char *name,JSObjectCallAsFunctionCallback callback){JSStringRef key=JSStringCreateWithUTF8CString(name);JSObjectSetProperty(c,JSContextGetGlobalObject(c),key,JSObjectMakeFunctionWithCallback(c,key,callback),kJSPropertyAttributeNone,nullptr);JSStringRelease(key);}
struct BindingKey {
 std::vector<uint32_t> sources,ids,versions;
 JSObjectRef entry=nullptr,dynamic=nullptr;
 JSValueRef packObject=nullptr;
 Pack *pack=nullptr;
 uint32_t nativeDynamicOffset=0;
 bool dynamicDirty=false;
 uint64_t usedEncoder=UINT64_MAX,bindingHash=0;
 uint64_t frame=0;
 uint32_t chunkId=0,packSize=0,dynamicCount=0;
 WGPUBindGroup group=nullptr;
 std::vector<JSObjectRef> used,dirty,proxyChecks;
 uint32_t proxyCoverage=0;
 std::vector<uint32_t> usedIds;
};
template<class Mark> static void stampResources(BindingKey &key,uint64_t encoder,bool cached,Mark mark){
 if(cached&&key.usedEncoder==encoder)return;
 for(size_t i=0;i<key.used.size();i++)if(!mark(i))return;
 key.usedEncoder=encoder;
}
// Every intermediate conversion is checked: a depth copy may have changed
// while its filtered child still carries the previous clean flag.
static bool proxiesReady(JSContextRef c,const BindingKey &key,uint32_t mask,JSValueRef *error){
 if(!mask)return true;
 if((key.proxyCoverage&mask)!=mask||key.proxyChecks.empty())return false;
 static JSStringRef dirty=JSStringCreateWithUTF8CString("dirty");
 for(JSObjectRef proxy:key.proxyChecks){
  JSValueRef value=JSObjectGetProperty(c,proxy,dirty,error);
  if((error&&*error)||!JSValueIsBoolean(c,value)||JSValueToBoolean(c,value))return false;
 }
 return true;
}
static JSClassRef bindingClass() {static JSClassRef value;static dispatch_once_t once;dispatch_once(&once,^{JSClassDefinition d=kJSClassDefinitionEmpty;d.className="CompiledBindingKey";d.finalize=[](JSObjectRef object){delete (BindingKey *)JSObjectGetPrivate(object);};value=JSClassCreate(&d);});return value;}
static uint32_t bound(uint32_t stage,uint32_t kind,uint32_t slot) {
 if(kind<2)return state[(stage*16+slot)*3+kind];
 return bindings[stage*152+(kind==2?0:kind==3?16:144)+slot];
}
static uint32_t version(uint32_t id){auto s=shadows.find(id);return s==shadows.end()?UINT32_MAX:s->second.version;}
static JSValueRef captureBindings(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error) {
 if(n<2)return JSValueMakeNull(c);uint32_t stage=number(c,v[1]);if(stage>=3)return JSValueMakeNull(c);
 JSObjectRef a=JSValueToObject(c,v[0],error);size_t bytes=JSObjectGetTypedArrayByteLength(c,a,error);uint32_t *src=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,a,error);
 if(bytes%8||bytes>1024||(!src&&bytes))return JSValueMakeNull(c);BindingKey *key=new BindingKey;key->sources.assign(src,src+bytes/4);
 for(size_t i=0;i<key->sources.size();i+=2){uint32_t kind=key->sources[i],slot=key->sources[i+1];if(kind>4||slot>=(kind<3?16:kind==3?128:8)){delete key;return JSValueMakeNull(c);}uint32_t id=bound(stage,kind,slot);key->ids.push_back(id);key->versions.push_back(kind==0?version(id):0);}
 return JSObjectMake(c,bindingClass(),key);
}
static JSValueRef bindingStatus(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error) {
 if(n<2)return JSValueMakeNumber(c,0);BindingKey *key=(BindingKey *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));uint32_t stage=number(c,v[1]);if(!key||stage>=3)return JSValueMakeNumber(c,0);
 bool versions=true;for(size_t i=0;i<key->ids.size();i++){uint32_t kind=key->sources[i*2],id=bound(stage,kind,key->sources[i*2+1]);if(id!=key->ids[i])return JSValueMakeNumber(c,0);if(kind==0&&version(id)!=key->versions[i])versions=false;}
 return JSValueMakeNumber(c,versions?3:1);
}
static JSValueRef updateBindingVersions(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error) {
 if(!n)return JSValueMakeUndefined(c);BindingKey *key=(BindingKey *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));if(key)for(size_t i=0;i<key->ids.size();i++)if(key->sources[i*2]==0)key->versions[i]=version(key->ids[i]);return JSValueMakeUndefined(c);
}
static constexpr uint32_t BindingCacheMaxKeys=128; // hit encoding reserves bit 8 for unchanged constants
static bool hashedBindingCache=true;
struct BindingCache {
 std::vector<uint32_t> sources;
 std::array<std::vector<BindingKey>,3> keys;
 std::array<std::unordered_map<uint64_t,std::vector<uint32_t>>,3> buckets;
 std::array<uint32_t,3> next{},last{UINT32_MAX,UINT32_MAX,UINT32_MAX};
 uint32_t epoch=UINT32_MAX;
 JSObjectRef retained=nullptr;
};
static uint64_t bindingResourceHash(BindingCache *cache,const uint32_t *ids){
 uint64_t hash=1469598103934665603ull;
 for(size_t i=0;i<cache->sources.size()/2;i++)if(cache->sources[i*2]>=2){hash^=ids[i];hash*=1099511628211ull;}
 return hash;
}
static void forgetBindingBucket(BindingCache *cache,uint32_t stage,uint32_t index){
 auto &buckets=cache->buckets[stage];auto found=buckets.find(cache->keys[stage][index].bindingHash);
 if(found==buckets.end())return;
 auto &entries=found->second;entries.erase(std::remove(entries.begin(),entries.end(),index),entries.end());
 if(entries.empty())buckets.erase(found);
}
static JSClassRef cacheClass(){static JSClassRef value;static dispatch_once_t once;dispatch_once(&once,^{JSClassDefinition d=kJSClassDefinitionEmpty;d.className="CompiledBindingCache";d.finalize=[](JSObjectRef object){delete (BindingCache *)JSObjectGetPrivate(object);};value=JSClassCreate(&d);});return value;}
static JSValueRef makeBindingCache(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(!n)return JSValueMakeNull(c);JSObjectRef a=JSValueToObject(c,v[0],error);size_t bytes=JSObjectGetTypedArrayByteLength(c,a,error);uint32_t *src=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,a,error);if(bytes%8||bytes>1024||(!src&&bytes))return JSValueMakeNull(c);
 BindingCache *cache=new BindingCache;if(bytes)cache->sources.assign(src,src+bytes/4);for(size_t i=0;i<cache->sources.size();i+=2){uint32_t kind=cache->sources[i],slot=cache->sources[i+1];if(kind>4||slot>=(kind<3?16:kind==3?128:8)){delete cache;return JSValueMakeNull(c);}}
 JSObjectRef object=JSObjectMake(c,cacheClass(),cache);cache->retained=JSObjectMakeArray(c,0,nullptr,nullptr);
 JSStringRef property=JSStringCreateWithUTF8CString("retainedEntries");JSObjectSetProperty(c,object,property,cache->retained,kJSPropertyAttributeNone,nullptr);JSStringRelease(property);return object;
}
static void cacheEpoch(JSContextRef c,BindingCache *cache,uint32_t epoch){
 if(cache->epoch==epoch)return;
 cache->epoch=epoch;for(auto &keys:cache->keys)keys.clear();for(auto &buckets:cache->buckets)buckets.clear();cache->next.fill(0);cache->last.fill(UINT32_MAX);
 // Numeric invalidation must release the JS owners too. Otherwise stale
 // bind groups and compressed texture shadows survive a streaming eviction.
 if(cache->retained){static JSStringRef length=JSStringCreateWithUTF8CString("length");JSObjectSetProperty(c,cache->retained,length,JSValueMakeNumber(c,0),kJSPropertyAttributeNone,nullptr);}
}
static uint32_t findCache(JSContextRef c,BindingCache *cache,uint32_t stage,uint32_t epoch){
 cacheEpoch(c,cache,epoch);uint32_t ids[128],versions[128];for(size_t i=0;i<cache->sources.size();i+=2){ids[i/2]=bound(stage,cache->sources[i],cache->sources[i+1]);versions[i/2]=cache->sources[i]==0?version(ids[i/2]):0;}
 auto &keys=cache->keys[stage];auto match=[&](uint32_t k)->uint32_t{const auto &key=keys[k];bool constants=true;for(size_t i=0;i<key.ids.size();i++){uint32_t kind=cache->sources[i*2];if(key.ids[i]!=ids[i]){if(kind<2)constants=false;else return 0;}if(kind==0&&versions[i]!=key.versions[i])constants=false;}cache->last[stage]=k;return (k+1)|(constants?256:0);};
 uint32_t recent=cache->last[stage];if(recent<keys.size()){uint32_t result=match(recent);if(result)return result;}
 if(hashedBindingCache){
  auto found=cache->buckets[stage].find(bindingResourceHash(cache,ids));
  if(found!=cache->buckets[stage].end())for(auto k=found->second.rbegin();k!=found->second.rend();k++)if(*k!=recent){uint32_t result=match(*k);if(result)return result;}
 }else for(uint32_t k=0;k<keys.size();k++)if(k!=recent){uint32_t result=match(k);if(result)return result;}
 return 0;
}
static JSValueRef findBindingCache(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(n<3)return JSValueMakeNumber(c,0);BindingCache *cache=(BindingCache *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));uint32_t stage=number(c,v[1]),epoch=number(c,v[2]);if(!cache||stage>=3)return JSValueMakeNumber(c,0);return JSValueMakeNumber(c,findCache(c,cache,stage,epoch));
}
static JSValueRef storeBindingCache(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(n<3)return JSValueMakeNumber(c,-1);BindingCache *cache=(BindingCache *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));uint32_t stage=number(c,v[1]),epoch=number(c,v[2]);if(!cache||stage>=3)return JSValueMakeNumber(c,-1);cacheEpoch(c,cache,epoch);
 auto &keys=cache->keys[stage];uint32_t capacity=hashedBindingCache?BindingCacheMaxKeys:32,index=keys.size()<capacity?(uint32_t)keys.size():cache->next[stage]++%capacity;if(index==keys.size())keys.emplace_back();else if(hashedBindingCache)forgetBindingBucket(cache,stage,index);auto &key=keys[index];key=BindingKey{};for(size_t i=0;i<cache->sources.size();i+=2){uint32_t kind=cache->sources[i],id=bound(stage,kind,cache->sources[i+1]);key.ids.push_back(id);key.versions.push_back(kind==0?version(id):0);}if(hashedBindingCache){key.bindingHash=bindingResourceHash(cache,key.ids.data());cache->buckets[stage][key.bindingHash].push_back(index);}cache->last[stage]=index;
 if(n>=6&&JSValueIsObject(c,v[3])){
  key.entry=JSValueToObject(c,v[3],error);auto property=[&](JSObjectRef object,const char *name){JSStringRef s=JSStringCreateWithUTF8CString(name);JSValueRef result=JSObjectGetProperty(c,object,s,error);JSStringRelease(s);return result;};
  key.frame=JSValueToNumber(c,property(key.entry,"frame"),nullptr);key.chunkId=number(c,property(key.entry,"chunkId"));key.packSize=number(c,v[5]);
  JSObjectRef result=JSValueToObject(c,property(key.entry,"result"),error);
  key.group=(WGPUBindGroup)rawGPUHandle(c,property(result,"bg"));
  key.dynamic=JSValueToObject(c,property(result,"dyn"),error);key.dynamicCount=number(c,property(key.dynamic,"length"));
  if(key.dynamicCount)key.nativeDynamicOffset=number(c,JSObjectGetPropertyAtIndex(c,key.dynamic,0,error));
  if(key.chunkId&&JSValueIsObjectOfClass(c,v[4],packClass())){key.packObject=v[4];key.pack=(Pack *)JSObjectGetPrivate(JSValueToObject(c,v[4],error));}
  auto resources=[&](const char *name,std::vector<JSObjectRef> &out){JSValueRef value=property(key.entry,name);if(!JSValueIsObject(c,value))return;JSObjectRef list=JSValueToObject(c,value,error);uint32_t count=number(c,property(list,"length"));for(uint32_t i=0;i<count;i++){JSValueRef item=JSObjectGetPropertyAtIndex(c,list,i,error);if(JSValueIsObject(c,item))out.push_back(JSValueToObject(c,item,error));}};
  resources("used",key.used);resources("dirty",key.dirty);resources("proxyChecks",key.proxyChecks);JSValueRef coverage=property(key.entry,"proxyCoverage");key.proxyCoverage=JSValueIsNumber(c,coverage)?number(c,coverage):0;
  for(JSObjectRef resource:key.used){JSValueRef id=property(resource,"nativeResourceId");key.usedIds.push_back(JSValueIsNumber(c,id)?number(c,id):0);}
  JSValueRef retained[]={v[3],v[4]};JSObjectSetPropertyAtIndex(c,cache->retained,stage*BindingCacheMaxKeys+index,JSObjectMakeArray(c,2,retained,error),error);
 }
 return JSValueMakeNumber(c,index);
}
static JSValueRef updateBindingCache(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(n<3)return JSValueMakeUndefined(c);BindingCache *cache=(BindingCache *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));uint32_t stage=number(c,v[1]),index=number(c,v[2]);if(cache&&stage<3&&index<cache->keys[stage].size()){auto &key=cache->keys[stage][index];for(size_t i=0;i<key.ids.size();i++){uint32_t kind=cache->sources[i*2];if(kind<2){key.ids[i]=bound(stage,kind,cache->sources[i*2+1]);if(kind==0)key.versions[i]=version(key.ids[i]);}}}return JSValueMakeUndefined(c);
}
static std::unordered_map<uint32_t,uint32_t> textureFlags;
struct Srv {uint32_t resource;bool buffer;};
static std::unordered_map<uint32_t,Srv> srvs;
struct ProxyPlan {std::vector<uint32_t> slots,samples;std::array<std::array<uint32_t,32>,3> ids;std::array<uint32_t,3> masks{},epochs{UINT32_MAX,UINT32_MAX,UINT32_MAX};};
static JSClassRef proxyClass(){static JSClassRef value;static dispatch_once_t once;dispatch_once(&once,^{JSClassDefinition d=kJSClassDefinitionEmpty;d.className="CompiledProxyPlan";d.finalize=[](JSObjectRef object){delete (ProxyPlan *)JSObjectGetPrivate(object);};value=JSClassCreate(&d);});return value;}
static JSValueRef registerTexture(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *){if(n>=2)textureFlags[number(c,v[0])]=number(c,v[1]);return JSValueMakeUndefined(c);}
static JSValueRef registerSrv(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *){if(n>=3)srvs[number(c,v[0])]={number(c,v[1]),JSValueToBoolean(c,v[2])};return JSValueMakeUndefined(c);}
static JSValueRef forgetProxy(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *){if(n){uint32_t id=number(c,v[0]);textureFlags.erase(id);srvs.erase(id);}return JSValueMakeUndefined(c);}
static JSValueRef makeProxyPlan(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(!n)return JSValueMakeNull(c);JSObjectRef a=JSValueToObject(c,v[0],error);size_t bytes=JSObjectGetTypedArrayByteLength(c,a,error);uint32_t *p=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,a,error);if(bytes%8||bytes>256||(!p&&bytes))return JSValueMakeNull(c);
 ProxyPlan *plan=new ProxyPlan;for(size_t i=0;i<bytes/4;i+=2){if(p[i]>=128){delete plan;return JSValueMakeNull(c);}plan->slots.push_back(p[i]);plan->samples.push_back(p[i+1]);}return JSObjectMake(c,proxyClass(),plan);
}
static uint32_t proxyMaskNative(ProxyPlan *plan,uint32_t stage,uint32_t epoch){
 if(!plan||stage>=3)return 0;
 if(plan->epochs[stage]!=epoch){plan->epochs[stage]=epoch;plan->ids[stage].fill(UINT32_MAX);plan->masks[stage]=0;}
 for(size_t i=0;i<plan->slots.size();i++){uint32_t id=bindings[stage*152+16+plan->slots[i]];if(plan->ids[stage][i]==id)continue;plan->ids[stage][i]=id;plan->masks[stage]&=~(1u<<i);
  auto srv=srvs.find(id);if(srv==srvs.end()||srv->second.buffer)continue;auto tex=textureFlags.find(srv->second.resource);if(tex==textureFlags.end())continue;
  uint32_t flags=tex->second,sample=plan->samples[i];bool depth=flags&2;if((depth&&sample!=1)||(!depth&&sample==1)||((flags&4)&&sample==0))plan->masks[stage]|=1u<<i;
 }return plan->masks[stage];
}
static JSValueRef proxyMask(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(n<3)return JSValueMakeNumber(c,0);
 ProxyPlan *plan=(ProxyPlan *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));
 return JSValueMakeNumber(c,proxyMaskNative(plan,number(c,v[1]),number(c,v[2])));
}
#include "NativeUniformRing.inc"
#include "NativeBindingDecoder.inc"
#include "NativePacketProfile.inc"
static void install(JSContext *context) {
 cacheUploadContents=[NSProcessInfo.processInfo.arguments containsObject:@"--native-constant-content-cache"]&&![NSProcessInfo.processInfo.arguments containsObject:@"--reference-constant-content-cache"];
 context[@"nativeConstantContentCache"]=@(cacheUploadContents);
 hashedBindingCache=![NSProcessInfo.processInfo.arguments containsObject:@"--reference-binding-cache"];
 context[@"nativeHashedBindingCache"]=@(hashedBindingCache);
 JSGlobalContextRef c=context.JSGlobalContextRef;JSObjectRef values=JSObjectMakeTypedArray(c,kJSTypedArrayTypeUint32Array,3*16*3,nullptr);state=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,values,nullptr);
 context[@"nativeCBState"]=[JSValue valueWithJSValueRef:values inContext:context];
 JSObjectRef bindingValues=JSObjectMakeTypedArray(c,kJSTypedArrayTypeUint32Array,3*152,nullptr);bindings=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,bindingValues,nullptr);
 context[@"nativeBindingState"]=[JSValue valueWithJSValueRef:bindingValues inContext:context];
 context[@"nativeCompiledUniforms"]=@(![NSProcessInfo.processInfo.arguments containsObject:@"--reference-packing"]);
 context[@"nativeDetailedProfile"]=@([NSProcessInfo.processInfo.arguments containsObject:@"--profile"]);
 installRing(context);
 installBindingDecoder(context);
 expose(c,"nativeProfilePackets",profilePackets);
 expose(c,"nativeMakeUniformBuffer",makeBuffer);expose(c,"nativeUploadUniform",upload);expose(c,"nativeForgetUniform",forget);expose(c,"nativeMakeUniformPack",makePack);expose(c,"nativeLookupUniformPack",lookup);expose(c,"nativeCopyUniformPack",copy);
 expose(c,"nativeCaptureBindings",captureBindings);expose(c,"nativeBindingStatus",bindingStatus);expose(c,"nativeUpdateBindingVersions",updateBindingVersions);
 expose(c,"nativeMakeBindingCache",makeBindingCache);expose(c,"nativeFindBindingCache",findBindingCache);expose(c,"nativeStoreBindingCache",storeBindingCache);expose(c,"nativeUpdateBindingCache",updateBindingCache);
 expose(c,"nativeRegisterTextureFlags",registerTexture);expose(c,"nativeRegisterSrv",registerSrv);expose(c,"nativeForgetProxy",forgetProxy);expose(c,"nativeMakeProxyPlan",makeProxyPlan);expose(c,"nativeProxyMask",proxyMask);
}
}
