#pragma once
#include <JavaScriptCore/JavaScriptCore.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>

struct PGTextureView {const uint8_t *bytes;size_t length;};
// The pointer is temporary: consume it before any further JavaScriptCore call.
static bool PGGetTextureView(JSContextRef context,JSValueRef value,PGTextureView *view){
 *view={nullptr,0};JSValueRef error=nullptr;
 if(!JSValueIsObject(context,value))return false;
 JSObjectRef input=JSValueToObject(context,value,&error),backing=nullptr;
 JSTypedArrayType type=JSValueGetTypedArrayType(context,value,&error);
 size_t offset=0,length=0,total=0;
 if(error)return false;
 if(type==kJSTypedArrayTypeArrayBuffer){backing=input;length=JSObjectGetArrayBufferByteLength(context,backing,&error);total=length;}
 else if(type!=kJSTypedArrayTypeNone){
  length=JSObjectGetTypedArrayByteLength(context,input,&error);
  offset=JSObjectGetTypedArrayByteOffset(context,input,&error);
  backing=JSObjectGetTypedArrayBuffer(context,input,&error);
  if(backing)total=JSObjectGetArrayBufferByteLength(context,backing,&error);
 }else{
  // DataView is a BufferSource but has no JSTypedArrayType of its own.
  static JSStringRef bufferKey=JSStringCreateWithUTF8CString("buffer"),offsetKey=JSStringCreateWithUTF8CString("byteOffset"),lengthKey=JSStringCreateWithUTF8CString("byteLength");
  JSValueRef buffer=JSObjectGetProperty(context,input,bufferKey,&error);
  if(error||JSValueGetTypedArrayType(context,buffer,&error)!=kJSTypedArrayTypeArrayBuffer)return false;
  backing=JSValueToObject(context,buffer,&error);total=JSObjectGetArrayBufferByteLength(context,backing,&error);
  JSValueRef offsetValue=JSObjectGetProperty(context,input,offsetKey,&error),lengthValue=JSObjectGetProperty(context,input,lengthKey,&error);
  if(error||!JSValueIsNumber(context,offsetValue)||!JSValueIsNumber(context,lengthValue))return false;
  double at=JSValueToNumber(context,offsetValue,&error),count=JSValueToNumber(context,lengthValue,&error);
  if(error||!isfinite(at)||!isfinite(count)||at<0||count<0||floor(at)!=at||floor(count)!=count||at>(double)total||count>(double)total-at)return false;
  offset=(size_t)at;length=(size_t)count;
 }
 if(error||!backing||offset>total||length>total-offset)return false;
 // Acquire the pointer LAST; preceding calls may relocate the backing store.
 const uint8_t *bytes=(const uint8_t *)JSObjectGetArrayBufferBytesPtr(context,backing,&error);
 if(error||(!bytes&&length))return false;
 view->bytes=bytes?bytes+offset:nullptr;view->length=length;return true;
}
