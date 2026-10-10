// Retain GPU objects until submission; replay state changes and draws in C++.
const immediateGPUCall = nativeGPUCall;
const drawBatch = new Float64Array(131072);
let drawBatchLength=0, drawBatchObjects=[], drawBatchIds=new Map();
let lastBatchObject=null,lastBatchObjectId=-1;
const batchObject=nativeFixedStatePacking?function(o){if(!o)return -1;if(o===lastBatchObject)return lastBatchObjectId;let i=drawBatchIds.get(o);if(i===undefined){i=drawBatchObjects.length;drawBatchIds.set(o,i);drawBatchObjects.push(o);}lastBatchObject=o;lastBatchObjectId=i;return i;}:function(o) {if(!o)return -1;let i=drawBatchIds.get(o);if(i===undefined){i=drawBatchObjects.length;drawBatchIds.set(o,i);drawBatchObjects.push(o);}return i;};
function flushDrawBatch() {if(!drawBatchLength)return;nativeGPUBatch(drawBatch.subarray(0,drawBatchLength),drawBatchObjects);drawBatchLength=0;drawBatchObjects=[];drawBatchIds.clear();lastBatchObject=null;lastBatchObjectId=-1;}
self.nativeGPUCall=function(op,object,args){flushDrawBatch();return immediateGPUCall(op,object,args)};
function batchHeader(op,object){if(drawBatchLength>drawBatch.length-256)flushDrawBatch();drawBatch[drawBatchLength++]=op;drawBatch[drawBatchLength++]=batchObject(object);}
// Frequent state packets have fixed widths. Avoid rest arrays and iterators on
// every state change; the reference branch keeps the original packing code.
const nativeDrawState=nativeFixedStatePacking?function(op,a,b,c,d,e,f,g){
 if(drawBatchLength>drawBatch.length-256)flushDrawBatch();
 let at=drawBatchLength;drawBatch[at++]=op;drawBatch[at++]=-1;
 switch(op){
  case 25:break;
  case 23:case 24:drawBatch[at++]=a;break;
  case 19:drawBatch[at++]=a;drawBatch[at++]=b;drawBatch[at++]=c;break;
  case 22:drawBatch[at++]=a;drawBatch[at++]=b;drawBatch[at++]=c;drawBatch[at++]=d;break;
  case 21:drawBatch[at++]=a;drawBatch[at++]=b;drawBatch[at++]=c;drawBatch[at++]=d;drawBatch[at++]=e;break;
  case 20:drawBatch[at++]=a;drawBatch[at++]=b;drawBatch[at++]=c;drawBatch[at++]=d;drawBatch[at++]=e;drawBatch[at++]=f;drawBatch[at++]=g;break;
  default:throw new Error('Unsupported native draw state operation.');
 }
 drawBatchLength=at;
}:function(op,...values){batchHeader(op,null);for(const value of values)drawBatch[drawBatchLength++]=value;};
const nativeDrawVertices=nativeFixedStatePacking?function(start,count,vertices){
 if(drawBatchLength>drawBatch.length-256)flushDrawBatch();
 let at=drawBatchLength;drawBatch[at++]=18;drawBatch[at++]=-1;drawBatch[at++]=start;drawBatch[at++]=count;
 for(let i=0;i<count;i++){const v=vertices[start+i];drawBatch[at++]=v.buf;drawBatch[at++]=v.stride;drawBatch[at++]=v.offset;}
 drawBatchLength=at;
}:function(start,count,vertices){batchHeader(18,null);drawBatch[drawBatchLength++]=start;drawBatch[drawBatchLength++]=count;for(let i=0;i<count;i++){const v=vertices[start+i];drawBatch[drawBatchLength++]=v.buf;drawBatch[drawBatchLength++]=v.stride;drawBatch[drawBatchLength++]=v.offset;}};
function nativeDrawPacket(pass,plan,kind,count,inst,start,base,first,encoder,width,height){batchHeader(26,pass);drawBatch[drawBatchLength++]=batchObject(plan);drawBatch[drawBatchLength++]=kind;drawBatch[drawBatchLength++]=count;drawBatch[drawBatchLength++]=inst;drawBatch[drawBatchLength++]=start;drawBatch[drawBatchLength++]=base;drawBatch[drawBatchLength++]=first;drawBatch[drawBatchLength++]=encoder;drawBatch[drawBatchLength++]=width;drawBatch[drawBatchLength++]=height;}
function reservePreparedDraw(){if(drawBatchLength>drawBatch.length-256)flushDrawBatch();}
function nativeDrawTransaction(pass,index){drawBatch[drawBatchLength++]=27;drawBatch[drawBatchLength++]=batchObject(pass);drawBatch[drawBatchLength++]=index;}
function nativeWarmTransaction(pass,index){drawBatch[drawBatchLength++]=28;drawBatch[drawBatchLength++]=batchObject(pass);drawBatch[drawBatchLength++]=index;}
const batchOps={'renderPassEncoder':{setPipeline:1,setBindGroup:3,draw:5,drawIndexed:6,drawIndirect:7,drawIndexedIndirect:8,setVertexBuffer:9,setIndexBuffer:10,setViewport:11,setScissorRect:12,setStencilReference:13,setBlendConstant:14,beginOcclusionQuery:15,endOcclusionQuery:16},'computePassEncoder':{setPipeline:2,setBindGroup:4,dispatchWorkgroups:17}};
for(const kind of Object.keys(batchOps)) for(const [method,op] of Object.entries(batchOps[kind])) {
 const previous=NativeGPU.prototype[method];
 NativeGPU.prototype[method]=function(...a) {
  if(this._type!==kind)return previous.apply(this,a);
  batchHeader(op,this);
  switch(op) {
   case 1:case 2:drawBatch[drawBatchLength++]=batchObject(a[0]);break;
   case 3:case 4:{let offsets=a[2]||[],start=a[3]||0,n=a[4]??offsets.length;drawBatch[drawBatchLength++]=a[0];drawBatch[drawBatchLength++]=batchObject(a[1]);drawBatch[drawBatchLength++]=n;for(let i=0;i<n;i++)drawBatch[drawBatchLength++]=offsets[start+i];break;}
   case 5:a=[a[0],a[1]??1,a[2]??0,a[3]??0];for(const x of a)drawBatch[drawBatchLength++]=x;break;
   case 6:a=[a[0],a[1]??1,a[2]??0,a[3]??0,a[4]??0];for(const x of a)drawBatch[drawBatchLength++]=x;break;
   case 7:case 8:drawBatch[drawBatchLength++]=batchObject(a[0]);drawBatch[drawBatchLength++]=a[1];break;
   case 9:drawBatch[drawBatchLength++]=a[0];drawBatch[drawBatchLength++]=batchObject(a[1]);drawBatch[drawBatchLength++]=a[2]??0;drawBatch[drawBatchLength++]=a[3]??-1;break;
   case 10:drawBatch[drawBatchLength++]=batchObject(a[0]);drawBatch[drawBatchLength++]=a[1]==='uint32'?1:0;drawBatch[drawBatchLength++]=a[2]??0;drawBatch[drawBatchLength++]=a[3]??-1;break;
   case 11:case 12:case 13:case 15:for(const x of a)drawBatch[drawBatchLength++]=x;break;
   case 14:{let v=normalizeGPUVector(a[0],'color');for(const k of ['r','g','b','a'])drawBatch[drawBatchLength++]=v[k];break;}
   case 16:break;
   case 17:drawBatch[drawBatchLength++]=a[0];drawBatch[drawBatchLength++]=a[1]??1;drawBatch[drawBatchLength++]=a[2]??1;break;
  }
 };
}
