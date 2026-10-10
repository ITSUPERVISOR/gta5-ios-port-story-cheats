#include "NativeRingPackets.h"
// Preparation updates CPU state only. Vertex/index changes and immutable draw
// snapshots replay at their original position inside the existing GPU batch.
namespace NativeWarm {
static uint32_t *auxiliary,*output;
static constexpr uint32_t NoRange=UINT32_MAX,VersionLimit=128,OutputWords=12+VersionLimit*2;
static constexpr size_t ReplayWordLimit=262144;
static std::vector<uint32_t> replayWords;
struct Range {uint32_t first,end;};
static std::vector<Range> ranges;
static void clear(){replayWords.clear();ranges.clear();}
static void replay(uint32_t range,WGPURenderPassEncoder pass){
 if(range>=ranges.size()){pg_log("Invalid native warm command range.");return;}
 const Range &r=ranges[range];uint32_t at=r.first;
 while(at<r.end){
  uint32_t op=replayWords[at++];
  if(op==18){uint32_t start=replayWords[at++],count=replayWords[at++];for(uint32_t i=0;i<count;i++){NativeDraw::vertices[start+i]={replayWords[at],replayWords[at+1],replayWords[at+2]};at+=3;}}
  else if(op==19){NativeDraw::indexId=replayWords[at++];NativeDraw::indexFormat=replayWords[at++];NativeDraw::indexOffset=replayWords[at++];}
  else if(op==27)NativePreparedDraw::replay(replayWords[at++],pass);
  else {pg_log("Invalid native warm replay operation.");return;}
 }
}
static uint32_t consume(JSContextRef c,uint64_t base,uint32_t tail,uint32_t head,uint32_t size,
                        const NativePreparedDraw::DrawContext &context,uint32_t limit,JSValueRef *error){
 std::fill(output,output+OutputWords,0);output[0]=tail;output[4]=output[10]=NoRange;
 if(!NativePipeline::state||!NativeUniforms::state||!NativeUniforms::bindings||!auxiliary||ranges.size()>=65536||replayWords.size()>ReplayWordLimit)return 0;
 NativeRing::Cursor cursor(engineMemory,engineMemorySize,base,tail,head,size);
 uint32_t first=replayWords.size();limit=std::min(limit,512u);
 for(uint32_t count=0;count<limit;count++){
  NativeRing::Packet packet;if(cursor.peek(packet)!=NativeRing::Status::Ready||!NativeRing::warmPayload(packet))break;
  const uint32_t *p=packet.payload;uint32_t op=packet.op;
  size_t replaySize=op==21?3+(size_t)p[1]*3:op==22?4:op==32?2:0;
  if(replaySize>ReplayWordLimit-replayWords.size())break;
  if(op==12||op==47){
   auto shadow=NativeUniforms::shadows.find(p[0]);
   // GPU buffers require the original flush/no-overwrite ordering. Large
   // uploads retain the existing frame-time accounting in the JS handler.
   if(shadow==NativeUniforms::shadows.end()||NativeDraw::buffer(p[0])||p[2]>=65536)break;
   if(p[1]>shadow->second.size||p[2]>shadow->second.size-p[1])break;
   uint32_t versionIndex=0;for(;versionIndex<output[9];versionIndex++)if(output[12+versionIndex*2]==p[0])break;
   if(versionIndex==VersionLimit)break;
   if(versionIndex==output[9]){output[12+versionIndex*2]=p[0];output[9]++;}
   if(!NativeUniforms::uploadNative(p[0],p[1],packet.wordIndex*4+12,p[2]))break;
   output[13+versionIndex*2]++;output[8]+=p[2];
  }else if(op==18){
   uint32_t stage=p[0],*destination=stage<2?NativePipeline::state+stage:auxiliary;
   if(*destination!=p[1])output[5]=1;
   *destination=p[1];output[6]|=1u<<stage;
  }else if(op==19||op==20){
   uint32_t slot=op==19?2:3;if(NativePipeline::state[slot]!=p[0])output[5]=1;
   NativePipeline::state[slot]=p[0];output[6]|=op==19?8:16;
  }else if(op==21){
   uint32_t start=p[0],amount=p[1];replayWords.push_back(18);replayWords.push_back(start);replayWords.push_back(amount);
   for(uint32_t i=0;i<amount;i++){
    uint32_t slot=start+i,id=p[2+i*3],stride=p[3+i*3],offset=p[4+i*3];uint32_t at=8+slot*2;
    if(NativePipeline::state[at+1]!=stride||!NativePipeline::state[at]!=!id)output[5]=1;
    NativePipeline::state[at]=id;NativePipeline::state[at+1]=stride;auxiliary[1+slot]=offset;output[7]|=1u<<slot;
    replayWords.insert(replayWords.end(),{id,stride,offset});
   }
  }else if(op==22){
   if(auxiliary[18]!=p[1]||!auxiliary[17]!=!p[0])output[5]=1;
   auxiliary[17]=p[0];auxiliary[18]=p[1];auxiliary[19]=p[2];NativePipeline::state[7]=p[0]?(p[1]==57?1:2):0;output[6]|=32;
   replayWords.insert(replayWords.end(),{19,p[0],p[1],p[2]});
  }else if(op>=23&&op<=25){
   uint32_t stage=p[0],start=p[1],amount=p[2],maximum=op==24?128:16;
   uint32_t copy=start<maximum?std::min(amount,maximum-start):0;
   if(copy){if(op==23)memcpy(NativeUniforms::state+(stage*16+start)*3,p+3,copy*12);else memcpy(NativeUniforms::bindings+stage*152+(op==24?16:0)+start,p+3,copy*4);}
  }else if(op==32){
   int32_t index=NativePreparedDraw::prepareNative(c,packet.wordIndex,context,error);
   if(index<0){output[10]=packet.offset;output[11]=1;break;}
   replayWords.insert(replayWords.end(),{27,(uint32_t)index});output[2]++;if(p[0]>=4)output[3]++;
  }else break;
  // peek() produced the exact published packet; never advance shared ring
  // publication here. JS publishes consumed tail at its usual yield boundary.
  if(!cursor.consume(packet)){pg_log("Invalid native warm ring advance.");break;}
  output[1]++;output[0]=cursor.tail();
 }
 if(replayWords.size()!=first){output[4]=ranges.size();ranges.push_back({first,(uint32_t)replayWords.size()});}
 return output[1];
}
static JSValueRef span(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *error){
 NativeCallClock timing(false,"native.warmSpan");
 if(output){std::fill(output,output+OutputWords,0);output[4]=output[10]=NoRange;}
 if(n<14||!output)return JSValueMakeNumber(c,0);
 double base=JSValueToNumber(c,v[0],error);if(!std::isfinite(base)||base<0||floor(base)!=base||base>(double)engineMemorySize)return JSValueMakeNumber(c,0);
 NativePreparedDraw::DrawContext context;
 context.tail=NativeUniforms::number(c,v[4]);context.epoch=NativeUniforms::number(c,v[5]);context.frame=JSValueToNumber(c,v[6],error);
 context.slot=NativeUniforms::number(c,v[7]);context.slots=NativeUniforms::number(c,v[8]);context.encoder=JSValueToNumber(c,v[9],error);context.encoderValue=v[9];
 context.resourceGeneration=NativeUniforms::number(c,v[10]);context.width=JSValueToNumber(c,v[11],error);context.height=JSValueToNumber(c,v[12],error);
 return JSValueMakeNumber(c,consume(c,base,NativeUniforms::number(c,v[1]),NativeUniforms::number(c,v[2]),NativeUniforms::number(c,v[3]),context,NativeUniforms::number(c,v[13]),error));
}
static void install(JSContext *context){
 JSContextRef c=context.JSGlobalContextRef;
 context[@"nativeWarmDispatcher"]=@(![NSProcessInfo.processInfo.arguments containsObject:@"--reference-warm-dispatch"]);
 for(auto pair:{std::pair<const char *,uint32_t **>{"nativeWarmState",&auxiliary},{"nativeWarmOutput",&output}}){
  uint32_t count=pair.second==&auxiliary?20:OutputWords;JSObjectRef array=JSObjectMakeTypedArray(c,kJSTypedArrayTypeUint32Array,count,nullptr);
  *pair.second=(uint32_t *)JSObjectGetTypedArrayBytesPtr(c,array,nullptr);context[[NSString stringWithUTF8String:pair.first]]=[JSValue valueWithJSValueRef:array inContext:context];
 }
 NativeUniforms::expose(c,"nativeConsumeWarmSpan",span);
}
}
