// Native graphics bookkeeping in JavaScriptCore. GPU work is executed by Dawn/Metal.
const self=globalThis;
const assetBase=typeof nativeAssetBase==='string'?nativeAssetBase:'';
self.location={origin:assetBase,search:'?low=1&syncpipelines=1',href:assetBase+'/'};
self.addEventListener=()=>{};
self.postMessage=(data)=>{if(data.ready)nativeLog('Native Metal renderer ready.');else if(data.error)nativeLog(String(data.error));};
const performance={now:()=>nativeNow(),timeOrigin:Date.now()-nativeNow()};
const console={log:(...v)=>nativeLog(v.join(' ')),warn:(...v)=>nativeLog(v.join(' ')),error:(...v)=>nativeLog(v.join(' '))};
const timerCancelled=new Set();let timerNext=1;
function setTimeout(callback,delay=0,...args){let id=timerNext++;nativeLater(()=>{if(!timerCancelled.delete(id))callback(...args)},delay);return id}
function clearTimeout(id){timerCancelled.add(id)}
function setInterval(callback,delay){let id=timerNext++;function tick(){if(timerCancelled.delete(id))return;callback();nativeLater(tick,delay)}nativeLater(tick,delay);return id}
function clearInterval(id){clearTimeout(id)}
class URLSearchParams {constructor(search=''){this.values=new Map(String(search).replace(/^\?/,'').split('&').filter(Boolean).map(x=>{let i=x.indexOf('=');return i<0?[decodeURIComponent(x),'']:[decodeURIComponent(x.slice(0,i)),decodeURIComponent(x.slice(i+1))]}))}get(k){return this.values.has(k)?this.values.get(k):null}has(k){return this.values.has(k)}}
class MessageChannel {constructor(){this.port1={onmessage:null};this.port2={postMessage:(data)=>nativeLater(()=>this.port1.onmessage?.({data}),0)}}}
class BroadcastChannel {constructor(name){this.name=name}postMessage(data){if(this.name==='game-progress'&&(data.label||data.error||data.gpuLost)){nativeLog(JSON.stringify(data));if(data.world)nativeLog('WORLD_READY');}}close(){}}
class TextDecoder {decode(bytes){return nativeDecode(bytes||new Uint8Array())}}
class TextEncoder {encode(text){return new Uint8Array(nativeEncode(String(text)))}}
function fetch(url,options={}){if(String(url).startsWith('/shot?')&&options.method==='POST'){nativeSaveShot(String(url),options.body);return Promise.resolve({ok:true,status:200})}return new Promise((resolve,reject)=>nativeFetch(String(url),options,(status,buffer,headers)=>resolve({ok:status>=200&&status<300,status,headers:{get:key=>Object.entries(headers).find(([k])=>k.toLowerCase()===key.toLowerCase())?.[1]||null},arrayBuffer:async()=>buffer,text:async()=>nativeDecode(new Uint8Array(buffer)),json:async()=>JSON.parse(nativeDecode(new Uint8Array(buffer)))}),reject))}
self.Atomics={
 load:(v,i)=>nativeAtomic('load',v.byteOffset+i*4,0),
 store:(v,i,n)=>nativeAtomic('store',v.byteOffset+i*4,n),
 add:(v,i,n)=>nativeAtomic('add',v.byteOffset+i*4,n),
 exchange:(v,i,n)=>nativeAtomic('exchange',v.byteOffset+i*4,n),
 notify:(v,i)=>nativeNotify(v.byteOffset+i*4),
 waitAsync:(v,i,n,timeout=100)=>{if(Atomics.load(v,i)!==n)return {async:false,value:'not-equal'};return {async:true,value:new Promise(resolve=>{let deadline=performance.now()+timeout;function poll(){if(Atomics.load(v,i)!==n)resolve('ok');else if(performance.now()>=deadline)resolve('timed-out');else nativeLater(poll,1)}nativeLater(poll,1)})}}
};
class NativeGPU {constructor(kind,native){this._type=kind;this._native=native;this._listeners={};if(kind==='device')this.lost=new Promise(()=>{})}addEventListener(event,listener){(this._listeners[event]||=([])).push(listener)}}
function wrapNativeGPU(kind,native){return new NativeGPU(kind,native)}
function normalizeGPUVector(v,kind){if(!Array.isArray(v))return v;let keys=kind==='color'?['r','g','b','a']:kind==='origin 3D'?['x','y','z']:['width','height','depthOrArrayLayers'];return Object.fromEntries(v.map((value,i)=>[keys[i],value]))}
function normalizeGPUBinding(v){let r=v.resource;if(r?._type==='sampler')return {...v,sampler:r};if(r?._type==='textureView')return {...v,textureView:r};return {...v,...r}}
for(let method of ['writeBuffer','writeTexture','submit','getMappedRange'])NativeGPU.prototype[method]=function(...args){return nativeGPUCall(this._type+'.'+method,this,args)};
NativeGPU.prototype.writeBuffer=function(buffer,offset,data,dataOffset=0,size){flushDrawBatch();return nativeWriteBuffer(this,buffer,offset,data,dataOffset,size)};
for(let method of ['mapAsync','onSubmittedWorkDone'])NativeGPU.prototype[method]=function(...args){return new Promise((resolve,reject)=>nativeGPUAsync(this._type+'.'+method,this,args,resolve,reject))};
let nativeCurrentDevice;
function nativeGPUError(message){for(const listener of nativeCurrentDevice?._listeners.uncapturederror||[])listener({error:{message}})}
const nativeCanvas={width:1280,height:720,getContext(){return {configure:()=>nativeGPUCall('canvas.configure',nativeCanvas,[]),getCurrentTexture:()=>nativeGPUCall('canvas.getCurrentTexture',nativeCanvas,[])}}};
self.navigator={gpu:{requestAdapter:async()=>({features:new Set(nativeFeatures),limits:nativeLimits,info:{description:nativeGPUDescription},requestDevice:async requirements=>{nativeCurrentDevice=nativeRequestDevice(requirements);if(nativeCurrentDevice)nativeCurrentDevice.features=new Set(requirements.requiredFeatures);return nativeCurrentDevice}}),getPreferredCanvasFormat:()=> 'bgra8unorm'}};
function startNativeRenderer(ring,size,dbg){return start({memory:{buffer:nativeMemory},ring,size,dbg:0,shaderUrl:assetBase+'/b/8b0b5899ed/shaders',canvas:nativeCanvas,shotEvery:0}).catch(e=>nativeLog('Native renderer loop stopped: '+(e.stack||e)))}

function nativeBCFormat(format){return format.startsWith('bc4')?'r8unorm':format.startsWith('bc5')?'rg8unorm':format.startsWith('bc6')?'rgba16float':format.endsWith('-srgb')?'rgba8unorm-srgb':'rgba8unorm';}
