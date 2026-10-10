"""Generate the native WebGPU descriptor bridge from Dawn's pinned API schema."""
from pathlib import Path
import json,re
root=Path(__file__).resolve().parents[1]
j=json.loads((root/'Tools/dawn/src/dawn/dawn.json').read_text())
header=(root/'Build/Dawn-iOS/gen/include/dawn/webgpu.h').read_text()
def pascal(n):return ''.join(w[0].upper()+w[1:] for w in n.split())
def camel(n):s=pascal(n);return s[0].lower()+s[1:]
def ctype(n):return 'WGPU'+pascal(n) if j.get(n,{}).get('category') not in (None,'native') else n
needed=set()
def need(n):
 if n in needed:return
 needed.add(n)
 if j.get(n,{}).get('category')=='structure':
  for m in j[n].get('members',[]):need(m['type'])
selected={
'device':['create bind group','create bind group layout','create buffer','create command encoder','create compute pipeline','create pipeline layout','create query set','create render pipeline','create sampler','create shader module','create texture'],
'texture':['create view','destroy'],
'buffer':['destroy','unmap'],
'render pipeline':['get bind group layout'],
'compute pipeline':['get bind group layout'],
'command encoder':['begin compute pass','begin render pass','clear buffer','copy buffer to buffer','copy buffer to texture','copy texture to buffer','copy texture to texture','finish','resolve query set'],
'render pass encoder':['begin occlusion query','draw','draw indexed','draw indexed indirect','draw indirect','end','end occlusion query','set bind group','set blend constant','set index buffer','set pipeline','set scissor rect','set stencil reference','set vertex buffer','set viewport'],
'compute pass encoder':['dispatch workgroups','end','set bind group','set pipeline']}
methods=[]
for obj,names in selected.items():
 for m in j[obj]['methods']:
  if m['name'] not in names:continue
  methods.append((obj,m))
  for a in m.get('args',[]):need(a['type'])
for n in ['texel copy buffer layout','texel copy texture info','extent 3D','limits','supported features']:need(n)
needed.discard('supported features')
# Native callback and chained extension structures are not descriptor fields here.
structures=sorted(n for n in needed if j.get(n,{}).get('category')=='structure' and n!='string view')
out=['// Generated from the pinned Dawn schema by Scripts/generate_gpu_bridge.py.']
for n in structures:out.append(f'static {ctype(n)} read{pascal(n)}(JSValue *, Arena &);')
def convert(n,v):
 cat=j.get(n,{}).get('category')
 if n=='string view':return f'a.string({v})'
 if cat=='structure':return f'read{pascal(n)}({v},a)'
 if cat=='object':return f'({ctype(n)})gpuHandle({v})'
 if cat=='enum':return f'enum{pascal(n)}({v})'
 return f'({ctype(n)})[{v} toDouble]'
for n in sorted(needed):
 if j.get(n,{}).get('category')!='enum':continue
 out.append(f'static {ctype(n)} enum{pascal(n)}(JSValue *v) {{ if(v.isNumber)return ({ctype(n)})v.toUInt32;NSString *s=v.toString;')
 for val in j[n]['values']:
  if val.get('valid')==False:continue
  chunks=val['name'].split();name=chunks[0].lower()
  for chunk in chunks[1:]:name+=('' if name[-1].isdigit() else '-')+chunk.lower()
  if 'jsrepr' in val:name=val['jsrepr'].strip("'")
  out.append(f' if([s isEqualToString:@{json.dumps(name)}])return ({ctype(n)}){val["value"]};')
 out.append(f' return ({ctype(n)})0; }}')
for n in structures:
 typ=ctype(n);macro='WGPU_'+'_'.join(n.upper().split())+'_INIT'
 if macro not in header:raise Exception(('missing init',macro))
 out.append(f'static {typ} read{pascal(n)}(JSValue *v,Arena &a){{ {typ} d={macro};if(!present(v))return d;')
 if n in ['extent 3D','origin 3D','color']:out.append(' v=normalizeVector(v,@"'+n+'");')
 if n=='bind group entry':out.append(' v=normalizeBinding(v);')
 arrays={m.get('length'):m for m in j[n]['members'] if isinstance(m.get('length'),str)}
 for m in j[n]['members']:
  field=camel(m['name']);prop='@'+json.dumps(field);v=f'v[{prop}]';annotation=m.get('annotation','');mt=m['type'];length=m.get('length')
  if m['name'] in arrays:continue
  if length:
   if not isinstance(length,str):raise Exception(m)
   counter=camel(length);typ2=ctype(mt)
   out.append(f' if(present({v})){{ JSValue *values={v};size_t n=values[@"length"].toUInt32;auto *p=a.alloc<{typ2}>(n);for(size_t i=0;i<n;i++)p[i]={convert(mt,"[values valueAtIndex:i]")};d.{field}=p;d.{counter}=n; }}')
  elif annotation:
   if mt=='void *':continue
   out.append(f' if(present({v})){{auto *p=a.alloc<{ctype(mt)}>();*p={convert(mt,v)};d.{field}=p;}}')
  else:out.append(f' if(present({v}))d.{field}={convert(mt,v)};')
 if n=='shader module descriptor':out.append(' auto *source=a.alloc<WGPUShaderSourceWGSL>();*source=WGPU_SHADER_SOURCE_WGSL_INIT;source->code=a.string(v[@"code"]);d.nextInChain=&source->chain;')
 if n=='texel copy buffer info':out.append(' d.layout=readTexelCopyBufferLayout(v,a);')
 out.append(' return d; }')
out.append('static JSValue *gpuGeneratedCall(NSString *op,JSValue *object,JSValue *args) { Arena a;JSContext *context=args.context;')
for obj,m in methods:
 op=camel(obj)+'.'+camel(m['name']);fn='wgpu'+pascal(obj)+pascal(m['name']);ret=m.get('returns','void');ret=ret.get('type') if isinstance(ret,dict) else ret
 args=m.get('args',[]);arrays={x.get('length'):x for x in args if isinstance(x.get('length'),str)};actual=[f'({ctype(obj)})gpuHandle(object)'];idx=0
 out.append(f' if([op isEqualToString:@"{op}"]){{')
 for k,arg in enumerate(args):
  mt=arg['type'];name=arg['name'];ann=arg.get('annotation');length=arg.get('length')
  if name in arrays:actual.append('0');continue # patched below when consuming array
  v=f'[args valueAtIndex:{idx}]';idx+=1;var=f'p{k}'
  if length:
   out.append(f' JSValue *list{k}={v};size_t count{k}=present(list{k})?list{k}[@"length"].toUInt32:0;auto *{var}=a.alloc<{ctype(mt)}>(count{k});for(size_t i=0;i<count{k};i++){var}[i]={convert(mt,f"[list{k} valueAtIndex:i]")};')
   actual[-1]=f'count{k}';actual.append(var)
  elif ann:
   out.append(f' auto {var}={convert(mt,v)};');actual.append('&'+var)
  else:
   expression=convert(mt,v)
   if 'default' in arg:
    default=str(arg['default']);default='WGPU_WHOLE_SIZE' if default=='whole size' else default
    if default in ('nullptr','undefined'):default='0'
    expression=f'(present({v})?{expression}:({ctype(mt)}){default})'
   actual.append(expression)
 call=fn+'('+','.join(actual)+')'
 if j.get(ret,{}).get('category')=='object':
  out.append(f' auto result={call};return wrapGPU(context,@"{camel(ret)}",result,[](void *p){{wgpu{pascal(ret)}Release(({ctype(ret)})p);}});')
 elif ret=='void':out.append(f' {call};return [JSValue valueWithUndefinedInContext:context];')
 else:out.append(f' auto result={call};return [JSValue valueWithDouble:(double)result inContext:context];')
 out.append(' }')
out+=[' return nil;','}']
(root/'Source/GeneratedGPUBridge.inc').write_text('\n'.join(out)+'\n')
js=['const gpuMethods='+json.dumps({camel(o):[camel(m['name']) for ob,m in methods if ob==o] for o in selected})+';']
js.append('for(const [kind,methods] of Object.entries(gpuMethods))for(const method of methods)NativeGPU.prototype[method]=function(...args){if(method==="setBindGroup"&&args.length>3){let [index,group,offsets,start,count]=args;args=[index,group,offsets.slice(start,start+count)];}const result=nativeGPUCall(this._type+"."+method,this,args);if(method==="createTexture"&&result){const d=args[0],size=normalizeGPUVector(d.size,"extent 3D");Object.assign(result,{format:d.format,width:size.width,height:size.height||1,depthOrArrayLayers:size.depthOrArrayLayers||1,mipLevelCount:d.mipLevelCount||1,sampleCount:d.sampleCount||1,dimension:d.dimension||"2d",usage:d.usage});}return result};')
(root/'Resources/gpu-methods.js').write_text('\n'.join(js)+'\n')
print(len(structures),'descriptor types;',len(methods),'native GPU methods')
