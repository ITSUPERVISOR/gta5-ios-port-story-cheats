#include <CommonCrypto/CommonDigest.h>
namespace ShaderCache {
static NSString *directory;
static NSCache<NSString *,NSData *> *memory;
static NSString *path(size_t size,const uint8_t *key){if(size>UINT32_MAX)return nil;unsigned char hash[CC_SHA256_DIGEST_LENGTH];CC_SHA256(key,(CC_LONG)size,hash);char hex[65];for(size_t i=0;i<sizeof hash;i++)snprintf(hex+i*2,3,"%02x",hash[i]);return [directory stringByAppendingPathComponent:[NSString stringWithUTF8String:hex]];}
static size_t load(size_t keySize,const uint8_t *key,size_t valueSize,uint8_t *value,void *,void *){@autoreleasepool {
 NSString *file=path(keySize,key);if(!file)return 0;NSData *data=[memory objectForKey:file];if(!data){data=[NSData dataWithContentsOfFile:file options:NSDataReadingMappedIfSafe error:nil];if(data)[memory setObject:data forKey:file cost:data.length];}if(!data)return 0;if(!value)return data.length;if(valueSize<data.length)return 0;memcpy(value,data.bytes,data.length);return data.length;
}}
static void save(size_t keySize,const uint8_t *key,size_t valueSize,const uint8_t *value,void *,void *){@autoreleasepool {
 if(valueSize>32*1024*1024)return;NSString *file=path(keySize,key);if(!file)return;NSData *data=[NSData dataWithBytes:value length:valueSize];[memory setObject:data forKey:file cost:data.length];[data writeToFile:file options:NSDataWritingAtomic error:nil];
}}
static WGPUDawnCacheDeviceDescriptor descriptor(){
 directory=[NSSearchPathForDirectoriesInDomains(NSCachesDirectory,NSUserDomainMask,YES).firstObject stringByAppendingPathComponent:@"DawnCache-85949168"];
 [NSFileManager.defaultManager createDirectoryAtPath:directory withIntermediateDirectories:YES attributes:nil error:nil];memory=[[NSCache alloc] init];memory.totalCostLimit=32*1024*1024;
 auto cache=WGPU_DAWN_CACHE_DEVICE_DESCRIPTOR_INIT;cache.isolationKey={"PlayGTA5-native-85949168-v1",WGPU_STRLEN};cache.dawnLoadCacheDataCallbackInfo.callback=load;cache.dawnStoreCacheDataCallbackInfo.callback=save;return cache;
}
}
