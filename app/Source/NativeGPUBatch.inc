// Replay draw state and draw calls in one native transition per command batch.
static void replayNativeBatch(JSContextRef context,JSObjectRef commands,JSObjectRef objects) {
 double started=CACurrentMediaTime();
 struct BatchClock {double start;~BatchClock(){nativeBatchMs+=(CACurrentMediaTime()-start)*1000;}} timing{started};
 struct SnapshotClock {JSContextRef c;~SnapshotClock(){NativeWarm::clear();NativePreparedDraw::clearSnapshots(c);}} cleanup{context};
 size_t bytes=JSObjectGetTypedArrayByteLength(context,commands,nullptr);
 if(bytes%sizeof(double))return;
 size_t offset=JSObjectGetTypedArrayByteOffset(context,commands,nullptr);
 JSObjectRef backing=JSObjectGetTypedArrayBuffer(context,commands,nullptr);
 const uint8_t *data=(const uint8_t *)JSObjectGetArrayBufferBytesPtr(context,backing,nullptr);
 if(!data)return;
 const double *p=(const double *)(data+offset),*end=p+bytes/sizeof(double);
 static JSStringRef lengthKey=JSStringCreateWithUTF8CString("length");
 uint32_t count=JSValueToNumber(context,JSObjectGetProperty(context,objects,lengthKey,nullptr),nullptr);
 std::vector<void *> handles;handles.reserve(count);
 for(uint32_t i=0;i<count;i++)handles.push_back(rawGPUHandle(context,JSObjectGetPropertyAtIndex(context,objects,i,nullptr)));
 auto handle=[&](double index)->void *{size_t i=(size_t)index;return i<handles.size()?handles[i]:nullptr;};
 while(p<end) {
  nativeBatchCommandCount++;
  uint32_t op=(uint32_t)*p++;if(end-p<1)return;void *target=handle(*p++);
  WGPURenderPassEncoder r=(WGPURenderPassEncoder)target;
  WGPUComputePassEncoder c=(WGPUComputePassEncoder)target;
  static const uint8_t operands[]={0,1,1,3,3,4,5,2,2,4,4,6,4,1,4,1,0,3,2,3,7,5,4,1,1,0,10,1,1};
  if(op>=sizeof(operands)||end-p<operands[op]){pg_log("Invalid native draw batch.");return;}
  if((op==3||op==4)&&(p[2]<0||p[2]>64||end-p<3+(size_t)p[2])){pg_log("Invalid native bind offsets.");return;}
  if(op==18&&(p[0]<0||p[1]<0||p[0]+p[1]>16||end-p<2+(size_t)p[1]*3)){pg_log("Invalid native vertex state.");return;}
  if(op>=1&&op<=4)NativePreparedDraw::invalidateReplay();
  switch(op) {
   case 1:wgpuRenderPassEncoderSetPipeline(r,(WGPURenderPipeline)handle(p[0]));p+=1;break;
   case 2:wgpuComputePassEncoderSetPipeline(c,(WGPUComputePipeline)handle(p[0]));p+=1;break;
   case 3:case 4:{uint32_t index=p[0];WGPUBindGroup bg=(WGPUBindGroup)handle(p[1]);uint32_t n=p[2];p+=3;std::vector<uint32_t> offsets;offsets.reserve(n);for(uint32_t i=0;i<n;i++)offsets.push_back(*p++);if(op==3)wgpuRenderPassEncoderSetBindGroup(r,index,bg,n,offsets.data());else wgpuComputePassEncoderSetBindGroup(c,index,bg,n,offsets.data());break;}
   case 5:wgpuRenderPassEncoderDraw(r,p[0],p[1],p[2],p[3]);p+=4;break;
   case 6:wgpuRenderPassEncoderDrawIndexed(r,p[0],p[1],p[2],(int32_t)p[3],p[4]);p+=5;break;
   case 7:wgpuRenderPassEncoderDrawIndirect(r,(WGPUBuffer)handle(p[0]),p[1]);p+=2;break;
   case 8:wgpuRenderPassEncoderDrawIndexedIndirect(r,(WGPUBuffer)handle(p[0]),p[1]);p+=2;break;
   case 9:wgpuRenderPassEncoderSetVertexBuffer(r,p[0],(WGPUBuffer)handle(p[1]),p[2],p[3]<0?WGPU_WHOLE_SIZE:(uint64_t)p[3]);p+=4;break;
   case 10:wgpuRenderPassEncoderSetIndexBuffer(r,(WGPUBuffer)handle(p[0]),p[1]?WGPUIndexFormat_Uint32:WGPUIndexFormat_Uint16,p[2],p[3]<0?WGPU_WHOLE_SIZE:(uint64_t)p[3]);p+=4;break;
   case 11:wgpuRenderPassEncoderSetViewport(r,p[0],p[1],p[2],p[3],p[4],p[5]);p+=6;break;
   case 12:wgpuRenderPassEncoderSetScissorRect(r,p[0],p[1],p[2],p[3]);p+=4;break;
   case 13:wgpuRenderPassEncoderSetStencilReference(r,p[0]);p+=1;break;
   case 14:{WGPUColor color={p[0],p[1],p[2],p[3]};wgpuRenderPassEncoderSetBlendConstant(r,&color);p+=4;break;}
   case 15:wgpuRenderPassEncoderBeginOcclusionQuery(r,p[0]);p+=1;break;
   case 16:wgpuRenderPassEncoderEndOcclusionQuery(r);break;
   case 17:wgpuComputePassEncoderDispatchWorkgroups(c,p[0],p[1],p[2]);p+=3;break;
   case 18:{uint32_t start=p[0],n=p[1];p+=2;for(uint32_t i=0;i<n;i++){NativeDraw::vertices[start+i]={(uint32_t)p[0],(uint32_t)p[1],(uint64_t)p[2]};p+=3;}break;}
   case 19:NativeDraw::indexId=p[0];NativeDraw::indexFormat=p[1];NativeDraw::indexOffset=p[2];p+=3;break;
   case 20:NativeDraw::hasViewport=p[0]!=0;for(uint32_t i=0;i<6;i++)NativeDraw::viewport[i]=p[i+1];p+=7;break;
   case 21:NativeDraw::hasScissor=p[0]!=0;for(uint32_t i=0;i<4;i++)NativeDraw::scissor[i]=p[i+1];p+=5;break;
   case 22:for(uint32_t i=0;i<4;i++)NativeDraw::blend[i]=p[i];p+=4;break;
   case 23:NativeDraw::stencil=p[0];p++;break;
   case 24:NativeDraw::rasterScissor=p[0]!=0;p++;break;
   case 25:NativeDraw::reset();break;
   case 26:NativeDraw::draw(r,(NativeDraw::Plan *)handle(p[0]),p+1);p+=10;break;
   case 27:NativePreparedDraw::replay((uint32_t)*p++,r);break;
   case 28:NativeWarm::replay((uint32_t)*p++,r);break;
   default:pg_log("Unknown native draw batch operation.");return;
  }
 }
}
static JSValueRef nativeBatch(JSContextRef context,JSObjectRef,JSObjectRef,size_t count,const JSValueRef values[],JSValueRef *error) {
 if(count<2||JSValueGetTypedArrayType(context,values[0],error)!=kJSTypedArrayTypeFloat64Array||!JSValueIsObject(context,values[1]))return JSValueMakeUndefined(context);
 replayNativeBatch(context,JSValueToObject(context,values[0],error),JSValueToObject(context,values[1],error));
 return JSValueMakeUndefined(context);
}
