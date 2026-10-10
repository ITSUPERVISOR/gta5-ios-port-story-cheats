// Parse stable attachment descriptions once. Each recipe owns the native
// views it references, and the renderer evicts recipes when resources die.
namespace NativePass {
struct Recipe {
 Arena arena;
 WGPURenderPassDescriptor descriptor;
 explicit Recipe(JSValue *value):descriptor(readRenderPassDescriptor(value,arena)) {
  for(size_t i=0;i<descriptor.colorAttachmentCount;i++) {
   const auto &a=descriptor.colorAttachments[i];
   if(a.view)wgpuTextureViewAddRef(a.view);
   if(a.resolveTarget)wgpuTextureViewAddRef(a.resolveTarget);
  }
  if(descriptor.depthStencilAttachment&&descriptor.depthStencilAttachment->view)wgpuTextureViewAddRef(descriptor.depthStencilAttachment->view);
  if(descriptor.occlusionQuerySet)wgpuQuerySetAddRef(descriptor.occlusionQuerySet);
  if(descriptor.timestampWrites&&descriptor.timestampWrites->querySet)wgpuQuerySetAddRef(descriptor.timestampWrites->querySet);
 }
 ~Recipe(){
  for(size_t i=0;i<descriptor.colorAttachmentCount;i++) {
   const auto &a=descriptor.colorAttachments[i];
   if(a.view)wgpuTextureViewRelease(a.view);
   if(a.resolveTarget)wgpuTextureViewRelease(a.resolveTarget);
  }
  if(descriptor.depthStencilAttachment&&descriptor.depthStencilAttachment->view)wgpuTextureViewRelease(descriptor.depthStencilAttachment->view);
  if(descriptor.occlusionQuerySet)wgpuQuerySetRelease(descriptor.occlusionQuerySet);
  if(descriptor.timestampWrites&&descriptor.timestampWrites->querySet)wgpuQuerySetRelease(descriptor.timestampWrites->querySet);
 }
 Recipe(const Recipe&)=delete;
 Recipe& operator=(const Recipe&)=delete;
};
static JSClassRef recipeClass(){static JSClassRef cls;static dispatch_once_t once;dispatch_once(&once,^{JSClassDefinition d=kJSClassDefinitionEmpty;d.className="NativeRenderPassRecipe";d.finalize=[](JSObjectRef object){delete (Recipe *)JSObjectGetPrivate(object);};cls=JSClassCreate(&d);});return cls;}
static JSValueRef prepare(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *) {
 NativeCallClock timing(false,"native.prepareRenderPass");
 if(!n)return JSValueMakeNull(c);
 Recipe *recipe=new Recipe([JSValue valueWithJSValueRef:v[0] inContext:graphicsContext]);
 return JSObjectMake(c,recipeClass(),recipe);
}
static JSValueRef begin(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error) {
 NativeCallClock timing(false,"native.beginRenderPass");
 if(n<2||!JSValueIsObjectOfClass(c,v[1],recipeClass()))return JSValueMakeNull(c);
 WGPUCommandEncoder encoder=(WGPUCommandEncoder)rawGPUHandle(c,v[0]);
 Recipe *recipe=(Recipe *)JSObjectGetPrivate(JSValueToObject(c,v[1],error));
 if(!encoder||!recipe)return JSValueMakeNull(c);
 WGPURenderPassEncoder pass=wgpuCommandEncoderBeginRenderPass(encoder,&recipe->descriptor);
 NativeDraw::invalidate();
 if(!pass)return JSValueMakeNull(c);
 JSObjectRef capsule=JSObjectMake(c,capsuleClass(),new Capsule{pass,[](void *p){wgpuRenderPassEncoderRelease((WGPURenderPassEncoder)p);}});
 static JSStringRef kind=JSStringCreateWithUTF8CString("renderPassEncoder"),wrap=JSStringCreateWithUTF8CString("wrapNativeGPU");
 JSObjectRef constructor=JSValueToObject(c,JSObjectGetProperty(c,JSContextGetGlobalObject(c),wrap,error),error);
 JSValueRef args[]={JSValueMakeString(c,kind),capsule};
 return JSObjectCallAsFunction(c,constructor,nullptr,2,args,error);
}
static void install(JSContext *context){
 context[@"nativePreparedPasses"]=@(![NSProcessInfo.processInfo.arguments containsObject:@"--reference-passes"]);
 NativeUniforms::expose(context.JSGlobalContextRef,"nativePrepareRenderPass",prepare);
 NativeUniforms::expose(context.JSGlobalContextRef,"nativeBeginRenderPass",begin);
}
}
