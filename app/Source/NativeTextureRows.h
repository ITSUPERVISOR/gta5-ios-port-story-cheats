#pragma once
#include "NativeTextureView.h"
#include <string.h>

static bool PGTextureRowRange(size_t length,size_t offset,size_t pitch,size_t count,size_t rows){
 if(!rows)return offset<=length;
 if(offset>length||count>length-offset)return false;
 return !pitch||rows-1<=(length-offset-count)/pitch;
}
// Destination and source spans are checked completely before changing any byte.
static bool PGCopyTextureRows(void *destination,size_t destinationLength,size_t destinationOffset,size_t destinationPitch,const void *source,size_t sourceLength,size_t sourceOffset,size_t sourcePitch,size_t rowBytes,size_t rows){
 if(!PGTextureRowRange(destinationLength,destinationOffset,destinationPitch,rowBytes,rows)||!PGTextureRowRange(sourceLength,sourceOffset,sourcePitch,rowBytes,rows))return false;
 if(!rows||!rowBytes)return true;
 if(!destination||!source)return false;
 uint8_t *dst=(uint8_t *)destination+destinationOffset;const uint8_t *src=(const uint8_t *)source+sourceOffset;
 // Preserve the original per-row overlap behavior for arbitrary callers.
 // Production texture shadows are separate from the engine allocation.
 if(destinationPitch==rowBytes&&sourcePitch==rowBytes){
  size_t bytes=rows*rowBytes;uintptr_t d=(uintptr_t)dst,s=(uintptr_t)src;
  if(d>=s?d-s>=bytes:s-d>=bytes){memcpy(dst,src,bytes);return true;}
 }
 for(size_t row=0;row<rows;row++)memmove(dst+row*destinationPitch,src+row*sourcePitch,rowBytes);
 return true;
}
static bool PGStoreTextureRows(JSContextRef context,size_t count,const JSValueRef arguments[],const void *engine,size_t engineSize,JSValueRef *error){
 if(count!=9)return false;
 size_t values[8];
 for(size_t i=0;i<8;i++){
  if(!JSValueIsNumber(context,arguments[i+1]))return false;
  double value=JSValueToNumber(context,arguments[i+1],error);
  if((error&&*error)||!isfinite(value)||value<0||floor(value)!=value||value>9007199254740991.0)return false;
  values[i]=(size_t)value;
 }
 size_t base=values[0],length=values[1],from=values[2],sourcePitch=values[3],to=values[4],destinationPitch=values[5],rowBytes=values[6],rows=values[7];
 if(!engine||base>engineSize||length>engineSize-base||!PGTextureRowRange(length,from,sourcePitch,rowBytes,rows))return false;
 PGTextureView destination;if(!PGGetTextureView(context,arguments[0],&destination))return false;
 // No JSC calls from here until the borrowed destination has been consumed.
 return PGCopyTextureRows((void *)destination.bytes,destination.length,to,destinationPitch,(const uint8_t *)engine+base,length,from,sourcePitch,rowBytes,rows);
}
