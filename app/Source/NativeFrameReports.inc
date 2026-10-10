// Optional aggregate performance reports (off unless launched with
// --frame-reports), sent to your own asset server. No asset contents, input, position or persistent device IDs.
#include "NativeFrameMetrics.h"
#include <mach/mach.h>
#include <os/proc.h>
namespace NativeFrameReports {
static bool enabled;
static PGFrameMetricsWindow window;
static double windowStart;
static uint64_t sequence;
static std::atomic<bool> inFlight{false},resetPending{false},applicationActive{true};
static std::atomic<unsigned> completedRequests{0},failedRequests{0};
static NSURLSession *session;
static NSString *runId,*build;
static uint64_t (*gpuMemorySample)()=nullptr;
static uint64_t (*memoryClampSample)()=nullptr;
static double field(JSContextRef c,JSValueRef value){return JSValueToNumber(c,value,nullptr);}
static NSString *number(double value){return [NSString stringWithFormat:@"%.3f",value];}
static void enqueue(PGFrameMetricsSnapshot stats){
 bool expected=false;if(!inFlight.compare_exchange_strong(expected,true,std::memory_order_relaxed))return;
 uint64_t seq=++sequence;
 dispatch_async(dispatch_get_global_queue(QOS_CLASS_UTILITY,0),^{@autoreleasepool {
  NSURLComponents *url=[NSURLComponents componentsWithString:[PGAssetServer() stringByAppendingString:@"/install/native-performance.json"]];
  double frames=(double)stats.frames;
#if TARGET_OS_IPHONE
  uint64_t headroom=os_proc_available_memory();
#else
  uint64_t headroom=0; // Host fixture has no iOS process memory allowance.
#endif
  task_vm_info_data_t memory={};mach_msg_type_number_t count=TASK_VM_INFO_COUNT;
  bool memoryKnown=task_info(mach_task_self(),TASK_VM_INFO,(task_info_t)&memory,&count)==KERN_SUCCESS;

  NSDictionary<NSString *,NSString *> *values=@{
   @"schema":@"1",@"qos":[NSProcessInfo.processInfo.arguments containsObject:@"--balanced-asset-qos"]?@"1":@"0",@"build":build,@"run":runId,@"seq":[NSString stringWithFormat:@"%llu",seq],
   @"frames":[NSString stringWithFormat:@"%llu",stats.frames],@"fps":number(stats.fps()),@"p95":number(stats.p95Ms),@"p99":number(stats.p99Ms),@"max":number(stats.maxMs),
   @"over33":[NSString stringWithFormat:@"%llu",stats.over33],@"over50":[NSString stringWithFormat:@"%llu",stats.over50],@"over100":[NSString stringWithFormat:@"%llu",stats.over100],@"over250":[NSString stringWithFormat:@"%llu",stats.over250],
   @"draws":number(stats.draws/frames),@"idle":number(stats.work.idle/frames),@"pipe":number(stats.work.pipeline/frames),@"modules":number(stats.work.modules/frames),@"upload":number(stats.work.uploads/frames),@"uploadMB":number(stats.work.uploadBytes/1048576.0),@"wait":number(stats.work.gpuWait/frames),@"submit":number(stats.work.submit/frames),
   @"worstIdle":number(stats.worstWork.idle),@"worstPipe":number(stats.worstWork.pipeline),@"worstModules":number(stats.worstWork.modules),@"worstUpload":number(stats.worstWork.uploads),@"worstWait":number(stats.worstWork.gpuWait),@"worstSubmit":number(stats.worstWork.submit),
   @"memoryClamps":number(memoryClampSample?memoryClampSample():0),@"metalMB":number(gpuMemorySample?gpuMemorySample()/1048576.0:0),@"footprintMB":number(memoryKnown?memory.phys_footprint/1048576.0:0),@"headroomMB":number(headroom/1048576.0),
   @"thermal":[NSString stringWithFormat:@"%ld",(long)NSProcessInfo.processInfo.thermalState]
  };
  NSMutableArray<NSURLQueryItem *> *items=[NSMutableArray arrayWithCapacity:values.count];for(NSString *key in [[values allKeys] sortedArrayUsingSelector:@selector(compare:)])[items addObject:[NSURLQueryItem queryItemWithName:key value:values[key]]];url.queryItems=items;
  NSMutableURLRequest *request=[NSMutableURLRequest requestWithURL:url.URL cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:3];
  NSURLSessionDataTask *task=[session dataTaskWithRequest:request completionHandler:^(NSData *,NSURLResponse *response,NSError *error){
   completedRequests.fetch_add(1,std::memory_order_relaxed);
   if(error||[(NSHTTPURLResponse *)response statusCode]!=200)failedRequests.fetch_add(1,std::memory_order_relaxed);
   inFlight.store(false,std::memory_order_relaxed);
  }];task.priority=NSURLSessionTaskPriorityLow;[task resume];
 }});
}
static void record(JSContextRef c,size_t n,const JSValueRef values[]){
 if(!enabled||n!=11)return;
 double time=field(c,values[0]);
 if(resetPending.exchange(false,std::memory_order_relaxed)){window.reset();windowStart=0;return;}
 if(!applicationActive.load(std::memory_order_relaxed)||!JSValueToBoolean(c,values[3]))return;
 double draws=field(c,values[2]),frame=field(c,values[1]);
 if(!std::isfinite(time)||!std::isfinite(draws)||draws<100||draws>UINT32_MAX)return;
 PGFrameWork work{field(c,values[4]),field(c,values[5]),field(c,values[6]),field(c,values[7]),field(c,values[8]),field(c,values[9]),field(c,values[10])};
 if(!window.record(frame,(uint64_t)draws,work))return;
 if(!windowStart)windowStart=time;
 if(time-windowStart>=15000){enqueue(window.snapshot());window.reset();windowStart=time;}
}
static void install(JSContext *context){
 enabled=[NSProcessInfo.processInfo.arguments containsObject:@"--frame-reports"];
 context[@"nativeFrameReports"]=@(enabled);if(!enabled){pg_log("Frame reports disabled.");return;}
 build=[NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleVersion"]?:@"probe";runId=NSUUID.UUID.UUIDString;
 NSURLSessionConfiguration *configuration=[NSURLSessionConfiguration ephemeralSessionConfiguration];configuration.URLCache=nil;configuration.HTTPCookieStorage=nil;configuration.HTTPMaximumConnectionsPerHost=1;configuration.timeoutIntervalForRequest=3;configuration.timeoutIntervalForResource=5;session=[NSURLSession sessionWithConfiguration:configuration];
#if TARGET_OS_IPHONE
 [NSNotificationCenter.defaultCenter addObserverForName:UIApplicationWillResignActiveNotification object:nil queue:nil usingBlock:^(NSNotification *){applicationActive.store(false,std::memory_order_relaxed);resetPending.store(true,std::memory_order_relaxed);}];
 [NSNotificationCenter.defaultCenter addObserverForName:UIApplicationDidBecomeActiveNotification object:nil queue:nil usingBlock:^(NSNotification *){resetPending.store(true,std::memory_order_relaxed);applicationActive.store(true,std::memory_order_relaxed);}];
#endif
 pg_log("Private frame reports enabled: bounded aggregates every 15 seconds of active world rendering.");
}
}
