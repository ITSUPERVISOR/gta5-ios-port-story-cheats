// Consume consecutive constant, texture and sampler binding packets directly
// from the published engine ring. No GPU commands or uploads are reordered.
static uint32_t *decodedBindings;
static JSValueRef decodeBindings(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *){
 if(n<4)return JSValueMakeNumber(c,0);
 uint64_t base=JSValueToNumber(c,v[0],nullptr);uint32_t tail=number(c,v[1]),head=number(c,v[2]),size=number(c,v[3]);
 decodedBindings[0]=tail;decodedBindings[1]=0;
 if(base>engineMemorySize||size>engineMemorySize-base||(base&3)||tail<64||tail>=size||head>size||(tail&3)||(head&3))return JSValueMakeNumber(c,0);
 uint32_t count=0;
 while(tail!=head&&tail<=size-8){
  const uint32_t *packet=(const uint32_t *)((const uint8_t *)engineMemory+base+tail);uint32_t op=packet[0],words=packet[1];
  if(op<23||op>25||words<3||words>(size-tail-8)/4)break;
  uint32_t end=tail+8+words*4;if(tail<head&&end>head)break;
  const uint32_t *p=packet+2;uint32_t stage=p[0],start=p[1],amount=p[2],limit=op==24?128:16;
  uint64_t required=3+(uint64_t)amount*(op==23?3:1);if(stage>=3||required>words)break;
  uint32_t copy=start<limit?std::min(amount,limit-start):0;
  if(copy){if(op==23)memcpy(state+(stage*16+start)*3,p+3,copy*3*sizeof(uint32_t));else memcpy(bindings+stage*152+(op==24?16:0)+start,p+3,copy*sizeof(uint32_t));}
  tail=end;count++;
 }
 decodedBindings[0]=tail;decodedBindings[1]=count;decodedBindings[2]+=count;return JSValueMakeNumber(c,count);
}
static void installBindingDecoder(JSContext *context){
 JSContextRef c=context.JSGlobalContextRef;JSObjectRef output=JSObjectMakeTypedArray(c,kJSTypedArrayTypeUint32Array,3,nullptr);decodedBindings=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,output,nullptr);context[@"nativeDecodedBindings"]=[JSValue valueWithJSValueRef:output inContext:context];expose(c,"nativeDecodeBindings",decodeBindings);
}
