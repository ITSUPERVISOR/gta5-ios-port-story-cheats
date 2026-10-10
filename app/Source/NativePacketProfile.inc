// Read-only profiling of the published ring. This never executes packets or
// advances the consumer tail; it measures whether native draw spans can help.
static std::array<uint64_t,48> packetCounts{};
static std::array<uint64_t,17> packetDrawSpans{};
static void profilePacketRange(uint64_t base,uint32_t tail,uint32_t head,uint32_t size){
 if(base>engineMemorySize||size>engineMemorySize-base||(base&3)||size<72||tail<64||tail>=size||head<64||head>size||(tail&3)||(head&3))return;
 uint32_t draws=0;uint64_t walked=0;
 auto finish=[&](){if(draws)packetDrawSpans[std::min(draws,16u)]++;draws=0;};
 while(tail!=head&&tail<=size-8&&walked<size){
  const uint32_t *p=(const uint32_t *)((const uint8_t *)engineMemory+base+tail);uint32_t op=p[0],n=p[1];
  if(op==1){if(tail<head)break;packetCounts[op]++;tail=64;walked+=8;continue;}
  if(n>(size-tail-8)/4)break;uint32_t end=tail+8+n*4;if(tail<head&&end>head)break;
  if(op<packetCounts.size())packetCounts[op]++;
  if(op==32)draws++;else if(op<23||op>25)finish();
  walked+=end-tail;tail=end;
 }
 finish();
}
static JSValueRef profilePackets(JSContextRef c,JSObjectRef,JSObjectRef,size_t n,const JSValueRef v[],JSValueRef *){
 if(nativeOperationProfile&&n>=4){NativeOperationClock timing("native.packetProfile");profilePacketRange(JSValueToNumber(c,v[0],nullptr),number(c,v[1]),number(c,v[2]),number(c,v[3]));}
 return JSValueMakeUndefined(c);
}
static void reportPackets(){
 NSMutableString *line=[NSMutableString stringWithString:@"Native packet profile: counts"];
 for(size_t i=0;i<packetCounts.size();i++)if(packetCounts[i])[line appendFormat:@" %zu=%llu",i,packetCounts[i]];
 [line appendString:@"; draw spans"];
 for(size_t i=1;i<packetDrawSpans.size();i++)if(packetDrawSpans[i])[line appendFormat:@" %zu%@=%llu",i,i==16?@"+":@"",packetDrawSpans[i]];
 pg_log(line.UTF8String);packetCounts.fill(0);packetDrawSpans.fill(0);
}
