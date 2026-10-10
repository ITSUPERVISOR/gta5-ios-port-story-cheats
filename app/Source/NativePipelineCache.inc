// Pipeline identity checks use shared numeric state; descriptor construction
// and content deduplication remain in the existing renderer on a cache miss.
namespace NativePipeline {
static uint32_t *state;
struct Layout {std::vector<uint32_t> slots;};
using Key=std::array<uint32_t,26>;
struct Hash {size_t operator()(const Key &key)const{uint64_t h=1469598103934665603ull;for(uint32_t value:key){h^=value;h*=1099511628211ull;}return h;}};
static std::unordered_map<Key,JSValueRef,Hash> values;
static JSObjectRef retained;
static uint32_t retainedCount;
static uint64_t generation;
static JSClassRef layoutClass(){static JSClassRef cls;static dispatch_once_t once;dispatch_once(&once,^{JSClassDefinition d=kJSClassDefinitionEmpty;d.className="NativePipelineLayoutSlots";d.finalize=[](JSObjectRef object){delete (Layout *)JSObjectGetPrivate(object);};cls=JSClassCreate(&d);});return cls;}
static JSValueRef makeLayout(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(!n)return JSValueMakeNull(c);JSObjectRef input=JSValueToObject(c,v[0],error);size_t bytes=JSObjectGetTypedArrayByteLength(c,input,error);uint32_t *data=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,input,error);if(bytes%4||bytes>64||(!data&&bytes))return JSValueMakeNull(c);
 Layout *layout=new Layout;for(size_t i=0;i<bytes/4;i++){if(data[i]>=16){delete layout;return JSValueMakeNull(c);}layout->slots.push_back(data[i]);}return JSObjectMake(c,layoutClass(),layout);
}
static Key key(Layout *layout,uint32_t tail){
 Key result{};for(uint32_t i=0;i<7;i++)result[i]=state[i];
 result[7]=(state[3]==3||state[3]==5)?state[7]:0;result[8]=tail;
 uint32_t i=9;for(uint32_t slot:layout->slots){uint32_t id=state[8+slot*2];result[i++]=NativeDraw::buffer(id)?state[9+slot*2]:0;}result[25]=i;return result;
}
static JSValueRef lookup(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(n<2)return JSValueMakeUndefined(c);Layout *layout=(Layout *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));if(!layout)return JSValueMakeUndefined(c);auto found=values.find(key(layout,NativeUniforms::number(c,v[1])));return found==values.end()?JSValueMakeUndefined(c):found->second;
}
static void clear(JSContextRef c){
 generation++;
 values.clear();retainedCount=0;retained=JSObjectMakeArray(c,0,nullptr,nullptr);
 JSStringRef name=JSStringCreateWithUTF8CString("nativePipelineValues");JSObjectSetProperty(c,JSContextGetGlobalObject(c),name,retained,kJSPropertyAttributeNone,nullptr);JSStringRelease(name);
}
static JSValueRef store(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 if(n<3)return JSValueMakeUndefined(c);Layout *layout=(Layout *)JSObjectGetPrivate(JSValueToObject(c,v[0],error));if(!layout)return JSValueMakeUndefined(c);if(values.size()>=65536)clear(c);
 Key identity=key(layout,NativeUniforms::number(c,v[1]));if(!values.contains(identity)){values.emplace(identity,v[2]);JSObjectSetPropertyAtIndex(c,retained,retainedCount++,v[2],error);}return JSValueMakeUndefined(c);
}
static void install(JSContext *context){
 JSContextRef c=context.JSGlobalContextRef;JSObjectRef shared=JSObjectMakeTypedArray(c,kJSTypedArrayTypeUint32Array,40,nullptr);state=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,shared,nullptr);state[3]=4;context[@"nativePipelineState"]=[JSValue valueWithJSValueRef:shared inContext:context];clear(c);
 NativeUniforms::expose(c,"nativeMakePipelineLayout",makeLayout);NativeUniforms::expose(c,"nativeLookupPipeline",lookup);NativeUniforms::expose(c,"nativeStorePipeline",store);
}
}
