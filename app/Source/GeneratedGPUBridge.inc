// Generated from the pinned Dawn schema by Scripts/generate_gpu_bridge.py.
static WGPUBindGroupDescriptor readBindGroupDescriptor(JSValue *, Arena &);
static WGPUBindGroupEntry readBindGroupEntry(JSValue *, Arena &);
static WGPUBindGroupLayoutDescriptor readBindGroupLayoutDescriptor(JSValue *, Arena &);
static WGPUBindGroupLayoutEntry readBindGroupLayoutEntry(JSValue *, Arena &);
static WGPUBlendComponent readBlendComponent(JSValue *, Arena &);
static WGPUBlendState readBlendState(JSValue *, Arena &);
static WGPUBufferBindingLayout readBufferBindingLayout(JSValue *, Arena &);
static WGPUBufferDescriptor readBufferDescriptor(JSValue *, Arena &);
static WGPUColor readColor(JSValue *, Arena &);
static WGPUColorTargetState readColorTargetState(JSValue *, Arena &);
static WGPUCommandBufferDescriptor readCommandBufferDescriptor(JSValue *, Arena &);
static WGPUCommandEncoderDescriptor readCommandEncoderDescriptor(JSValue *, Arena &);
static WGPUComputePassDescriptor readComputePassDescriptor(JSValue *, Arena &);
static WGPUComputePipelineDescriptor readComputePipelineDescriptor(JSValue *, Arena &);
static WGPUComputeState readComputeState(JSValue *, Arena &);
static WGPUConstantEntry readConstantEntry(JSValue *, Arena &);
static WGPUDepthStencilState readDepthStencilState(JSValue *, Arena &);
static WGPUExtent3D readExtent3D(JSValue *, Arena &);
static WGPUFragmentState readFragmentState(JSValue *, Arena &);
static WGPULimits readLimits(JSValue *, Arena &);
static WGPUMultisampleState readMultisampleState(JSValue *, Arena &);
static WGPUOrigin3D readOrigin3D(JSValue *, Arena &);
static WGPUPassTimestampWrites readPassTimestampWrites(JSValue *, Arena &);
static WGPUPipelineLayoutDescriptor readPipelineLayoutDescriptor(JSValue *, Arena &);
static WGPUPrimitiveState readPrimitiveState(JSValue *, Arena &);
static WGPUQuerySetDescriptor readQuerySetDescriptor(JSValue *, Arena &);
static WGPURenderPassColorAttachment readRenderPassColorAttachment(JSValue *, Arena &);
static WGPURenderPassDepthStencilAttachment readRenderPassDepthStencilAttachment(JSValue *, Arena &);
static WGPURenderPassDescriptor readRenderPassDescriptor(JSValue *, Arena &);
static WGPURenderPipelineDescriptor readRenderPipelineDescriptor(JSValue *, Arena &);
static WGPUSamplerBindingLayout readSamplerBindingLayout(JSValue *, Arena &);
static WGPUSamplerDescriptor readSamplerDescriptor(JSValue *, Arena &);
static WGPUShaderModuleDescriptor readShaderModuleDescriptor(JSValue *, Arena &);
static WGPUStencilFaceState readStencilFaceState(JSValue *, Arena &);
static WGPUStorageTextureBindingLayout readStorageTextureBindingLayout(JSValue *, Arena &);
static WGPUTexelCopyBufferInfo readTexelCopyBufferInfo(JSValue *, Arena &);
static WGPUTexelCopyBufferLayout readTexelCopyBufferLayout(JSValue *, Arena &);
static WGPUTexelCopyTextureInfo readTexelCopyTextureInfo(JSValue *, Arena &);
static WGPUTextureBindingLayout readTextureBindingLayout(JSValue *, Arena &);
static WGPUTextureDescriptor readTextureDescriptor(JSValue *, Arena &);
static WGPUTextureViewDescriptor readTextureViewDescriptor(JSValue *, Arena &);
static WGPUVertexAttribute readVertexAttribute(JSValue *, Arena &);
static WGPUVertexBufferLayout readVertexBufferLayout(JSValue *, Arena &);
static WGPUVertexState readVertexState(JSValue *, Arena &);
static WGPUAddressMode enumAddressMode(JSValue *v) { if(v.isNumber)return (WGPUAddressMode)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUAddressMode)0;
 if([s isEqualToString:@"clamp-to-edge"])return (WGPUAddressMode)1;
 if([s isEqualToString:@"repeat"])return (WGPUAddressMode)2;
 if([s isEqualToString:@"mirror-repeat"])return (WGPUAddressMode)3;
 return (WGPUAddressMode)0; }
static WGPUBlendFactor enumBlendFactor(JSValue *v) { if(v.isNumber)return (WGPUBlendFactor)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUBlendFactor)0;
 if([s isEqualToString:@"zero"])return (WGPUBlendFactor)1;
 if([s isEqualToString:@"one"])return (WGPUBlendFactor)2;
 if([s isEqualToString:@"src"])return (WGPUBlendFactor)3;
 if([s isEqualToString:@"one-minus-src"])return (WGPUBlendFactor)4;
 if([s isEqualToString:@"src-alpha"])return (WGPUBlendFactor)5;
 if([s isEqualToString:@"one-minus-src-alpha"])return (WGPUBlendFactor)6;
 if([s isEqualToString:@"dst"])return (WGPUBlendFactor)7;
 if([s isEqualToString:@"one-minus-dst"])return (WGPUBlendFactor)8;
 if([s isEqualToString:@"dst-alpha"])return (WGPUBlendFactor)9;
 if([s isEqualToString:@"one-minus-dst-alpha"])return (WGPUBlendFactor)10;
 if([s isEqualToString:@"src-alpha-saturated"])return (WGPUBlendFactor)11;
 if([s isEqualToString:@"constant"])return (WGPUBlendFactor)12;
 if([s isEqualToString:@"one-minus-constant"])return (WGPUBlendFactor)13;
 if([s isEqualToString:@"src1"])return (WGPUBlendFactor)14;
 if([s isEqualToString:@"one-minus-src1"])return (WGPUBlendFactor)15;
 if([s isEqualToString:@"src1-alpha"])return (WGPUBlendFactor)16;
 if([s isEqualToString:@"one-minus-src1-alpha"])return (WGPUBlendFactor)17;
 return (WGPUBlendFactor)0; }
static WGPUBlendOperation enumBlendOperation(JSValue *v) { if(v.isNumber)return (WGPUBlendOperation)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUBlendOperation)0;
 if([s isEqualToString:@"add"])return (WGPUBlendOperation)1;
 if([s isEqualToString:@"subtract"])return (WGPUBlendOperation)2;
 if([s isEqualToString:@"reverse-subtract"])return (WGPUBlendOperation)3;
 if([s isEqualToString:@"min"])return (WGPUBlendOperation)4;
 if([s isEqualToString:@"max"])return (WGPUBlendOperation)5;
 return (WGPUBlendOperation)0; }
static WGPUBufferBindingType enumBufferBindingType(JSValue *v) { if(v.isNumber)return (WGPUBufferBindingType)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUBufferBindingType)0;
 if([s isEqualToString:@"uniform"])return (WGPUBufferBindingType)2;
 if([s isEqualToString:@"storage"])return (WGPUBufferBindingType)3;
 if([s isEqualToString:@"read-only-storage"])return (WGPUBufferBindingType)4;
 return (WGPUBufferBindingType)0; }
static WGPUCompareFunction enumCompareFunction(JSValue *v) { if(v.isNumber)return (WGPUCompareFunction)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUCompareFunction)0;
 if([s isEqualToString:@"never"])return (WGPUCompareFunction)1;
 if([s isEqualToString:@"less"])return (WGPUCompareFunction)2;
 if([s isEqualToString:@"equal"])return (WGPUCompareFunction)3;
 if([s isEqualToString:@"less-equal"])return (WGPUCompareFunction)4;
 if([s isEqualToString:@"greater"])return (WGPUCompareFunction)5;
 if([s isEqualToString:@"not-equal"])return (WGPUCompareFunction)6;
 if([s isEqualToString:@"greater-equal"])return (WGPUCompareFunction)7;
 if([s isEqualToString:@"always"])return (WGPUCompareFunction)8;
 return (WGPUCompareFunction)0; }
static WGPUCullMode enumCullMode(JSValue *v) { if(v.isNumber)return (WGPUCullMode)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUCullMode)0;
 if([s isEqualToString:@"none"])return (WGPUCullMode)1;
 if([s isEqualToString:@"front"])return (WGPUCullMode)2;
 if([s isEqualToString:@"back"])return (WGPUCullMode)3;
 return (WGPUCullMode)0; }
static WGPUFeatureName enumFeatureName(JSValue *v) { if(v.isNumber)return (WGPUFeatureName)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"core-features-and-limits"])return (WGPUFeatureName)1;
 if([s isEqualToString:@"depth-clip-control"])return (WGPUFeatureName)2;
 if([s isEqualToString:@"depth32float-stencil8"])return (WGPUFeatureName)3;
 if([s isEqualToString:@"texture-compression-bc"])return (WGPUFeatureName)4;
 if([s isEqualToString:@"texture-compression-bc-sliced-3d"])return (WGPUFeatureName)5;
 if([s isEqualToString:@"texture-compression-etc2"])return (WGPUFeatureName)6;
 if([s isEqualToString:@"texture-compression-astc"])return (WGPUFeatureName)7;
 if([s isEqualToString:@"texture-compression-astc-sliced-3d"])return (WGPUFeatureName)8;
 if([s isEqualToString:@"timestamp-query"])return (WGPUFeatureName)9;
 if([s isEqualToString:@"indirect-first-instance"])return (WGPUFeatureName)10;
 if([s isEqualToString:@"shader-f16"])return (WGPUFeatureName)11;
 if([s isEqualToString:@"rg11b10ufloat-renderable"])return (WGPUFeatureName)12;
 if([s isEqualToString:@"bgra8unorm-storage"])return (WGPUFeatureName)13;
 if([s isEqualToString:@"float32-filterable"])return (WGPUFeatureName)14;
 if([s isEqualToString:@"float32-blendable"])return (WGPUFeatureName)15;
 if([s isEqualToString:@"clip-distances"])return (WGPUFeatureName)16;
 if([s isEqualToString:@"dual-source-blending"])return (WGPUFeatureName)17;
 if([s isEqualToString:@"subgroups"])return (WGPUFeatureName)18;
 if([s isEqualToString:@"texture-formats-tier1"])return (WGPUFeatureName)19;
 if([s isEqualToString:@"texture-formats-tier2"])return (WGPUFeatureName)20;
 if([s isEqualToString:@"primitive-index"])return (WGPUFeatureName)21;
 if([s isEqualToString:@"texture-component-swizzle"])return (WGPUFeatureName)22;
 if([s isEqualToString:@"subgroup-size-control"])return (WGPUFeatureName)23;
 if([s isEqualToString:@"texture-compression-unaligned"])return (WGPUFeatureName)24;
 if([s isEqualToString:@"atomic-vec2u-min-max"])return (WGPUFeatureName)25;
 if([s isEqualToString:@"dawn-internal-usages"])return (WGPUFeatureName)0;
 if([s isEqualToString:@"dawn-multi-planar-formats"])return (WGPUFeatureName)1;
 if([s isEqualToString:@"dawn-native"])return (WGPUFeatureName)2;
 if([s isEqualToString:@"chromium-experimental-timestamp-query-inside-passes"])return (WGPUFeatureName)3;
 if([s isEqualToString:@"implicit-device-synchronization"])return (WGPUFeatureName)4;
 if([s isEqualToString:@"transient-attachments"])return (WGPUFeatureName)6;
 if([s isEqualToString:@"msaa-render-to-single-sampled"])return (WGPUFeatureName)7;
 if([s isEqualToString:@"d3d11multithread-protected"])return (WGPUFeatureName)8;
 if([s isEqualToString:@"angle-texture-sharing"])return (WGPUFeatureName)9;
 if([s isEqualToString:@"pixel-local-storage-coherent"])return (WGPUFeatureName)10;
 if([s isEqualToString:@"pixel-local-storage-non-coherent"])return (WGPUFeatureName)11;
 if([s isEqualToString:@"chromium-experimental-unorm16-texture-formats"])return (WGPUFeatureName)12;
 if([s isEqualToString:@"multi-planar-format-extended-usages"])return (WGPUFeatureName)13;
 if([s isEqualToString:@"multi-planar-format-p010"])return (WGPUFeatureName)14;
 if([s isEqualToString:@"host-mapped-pointer"])return (WGPUFeatureName)15;
 if([s isEqualToString:@"multi-planar-render-targets"])return (WGPUFeatureName)16;
 if([s isEqualToString:@"multi-planar-format-nv12a"])return (WGPUFeatureName)17;
 if([s isEqualToString:@"framebuffer-fetch"])return (WGPUFeatureName)18;
 if([s isEqualToString:@"buffer-map-extended-usages"])return (WGPUFeatureName)19;
 if([s isEqualToString:@"adapter-properties-memory-heaps"])return (WGPUFeatureName)20;
 if([s isEqualToString:@"adapter-properties-d3d"])return (WGPUFeatureName)21;
 if([s isEqualToString:@"adapter-properties-vk"])return (WGPUFeatureName)22;
 if([s isEqualToString:@"dawn-format-capabilities"])return (WGPUFeatureName)23;
 if([s isEqualToString:@"dawn-drm-format-capabilities"])return (WGPUFeatureName)24;
 if([s isEqualToString:@"multi-planar-format-nv16"])return (WGPUFeatureName)25;
 if([s isEqualToString:@"multi-planar-format-nv24"])return (WGPUFeatureName)26;
 if([s isEqualToString:@"multi-planar-format-p210"])return (WGPUFeatureName)27;
 if([s isEqualToString:@"multi-planar-format-p410"])return (WGPUFeatureName)28;
 if([s isEqualToString:@"shared-texture-memory-vk-dedicated-allocation"])return (WGPUFeatureName)29;
 if([s isEqualToString:@"shared-texture-memory-a-hardware-buffer"])return (WGPUFeatureName)30;
 if([s isEqualToString:@"shared-texture-memory-dma-buf"])return (WGPUFeatureName)31;
 if([s isEqualToString:@"shared-texture-memory-opaque-fd"])return (WGPUFeatureName)32;
 if([s isEqualToString:@"shared-texture-memory-zircon-handle"])return (WGPUFeatureName)33;
 if([s isEqualToString:@"shared-texture-memory-dxgi-shared-handle"])return (WGPUFeatureName)34;
 if([s isEqualToString:@"shared-texture-memory-d3d11texture-2d"])return (WGPUFeatureName)35;
 if([s isEqualToString:@"shared-texture-memory-io-surface"])return (WGPUFeatureName)36;
 if([s isEqualToString:@"shared-texture-memory-egl-image"])return (WGPUFeatureName)37;
 if([s isEqualToString:@"shared-fence-vk-semaphore-opaque-fd"])return (WGPUFeatureName)38;
 if([s isEqualToString:@"shared-fence-sync-fd"])return (WGPUFeatureName)39;
 if([s isEqualToString:@"shared-fence-vk-semaphore-zircon-handle"])return (WGPUFeatureName)40;
 if([s isEqualToString:@"shared-fence-dxgi-shared-handle"])return (WGPUFeatureName)41;
 if([s isEqualToString:@"shared-fence-mtl-shared-event"])return (WGPUFeatureName)42;
 if([s isEqualToString:@"shared-buffer-memory-d3d12resource"])return (WGPUFeatureName)43;
 if([s isEqualToString:@"static-samplers"])return (WGPUFeatureName)44;
 if([s isEqualToString:@"y-cb-cr-vulkan-samplers"])return (WGPUFeatureName)45;
 if([s isEqualToString:@"shader-module-compilation-options"])return (WGPUFeatureName)46;
 if([s isEqualToString:@"dawn-load-resolve-texture"])return (WGPUFeatureName)47;
 if([s isEqualToString:@"dawn-partial-load-resolve-texture"])return (WGPUFeatureName)48;
 if([s isEqualToString:@"chromium-experimental-multi-draw-indirect"])return (WGPUFeatureName)49;
 if([s isEqualToString:@"dawn-texel-copy-buffer-row-alignment"])return (WGPUFeatureName)50;
 if([s isEqualToString:@"flexible-texture-views"])return (WGPUFeatureName)51;
 if([s isEqualToString:@"chromium-experimental-subgroup-matrix"])return (WGPUFeatureName)52;
 if([s isEqualToString:@"shared-fence-egl-sync"])return (WGPUFeatureName)53;
 if([s isEqualToString:@"dawn-device-allocator-control"])return (WGPUFeatureName)54;
 if([s isEqualToString:@"adapter-properties-wgpu"])return (WGPUFeatureName)55;
 if([s isEqualToString:@"shared-buffer-memory-from-windows-handle"])return (WGPUFeatureName)56;
 if([s isEqualToString:@"shared-texture-memory-d3d12resource"])return (WGPUFeatureName)57;
 if([s isEqualToString:@"chromium-experimental-sampling-resource-table"])return (WGPUFeatureName)58;
 if([s isEqualToString:@"unorm16formats-for-external-texture"])return (WGPUFeatureName)59;
 if([s isEqualToString:@"opaque-y-cb-cr-android-for-external-texture"])return (WGPUFeatureName)60;
 if([s isEqualToString:@"unorm16filterable"])return (WGPUFeatureName)61;
 if([s isEqualToString:@"render-pass-render-area"])return (WGPUFeatureName)62;
 if([s isEqualToString:@"adapter-properties-drm"])return (WGPUFeatureName)63;
 if([s isEqualToString:@"dawn-allow-undefined-load-store-op"])return (WGPUFeatureName)64;
 if([s isEqualToString:@"buffer-map-write-extended-usages"])return (WGPUFeatureName)65;
 if([s isEqualToString:@"shared-buffer-memory-host-pointer"])return (WGPUFeatureName)66;
 return (WGPUFeatureName)0; }
static WGPUFilterMode enumFilterMode(JSValue *v) { if(v.isNumber)return (WGPUFilterMode)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUFilterMode)0;
 if([s isEqualToString:@"nearest"])return (WGPUFilterMode)1;
 if([s isEqualToString:@"linear"])return (WGPUFilterMode)2;
 return (WGPUFilterMode)0; }
static WGPUFrontFace enumFrontFace(JSValue *v) { if(v.isNumber)return (WGPUFrontFace)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUFrontFace)0;
 if([s isEqualToString:@"ccw"])return (WGPUFrontFace)1;
 if([s isEqualToString:@"cw"])return (WGPUFrontFace)2;
 return (WGPUFrontFace)0; }
static WGPUIndexFormat enumIndexFormat(JSValue *v) { if(v.isNumber)return (WGPUIndexFormat)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUIndexFormat)0;
 if([s isEqualToString:@"uint16"])return (WGPUIndexFormat)1;
 if([s isEqualToString:@"uint32"])return (WGPUIndexFormat)2;
 return (WGPUIndexFormat)0; }
static WGPULoadOp enumLoadOp(JSValue *v) { if(v.isNumber)return (WGPULoadOp)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPULoadOp)0;
 if([s isEqualToString:@"load"])return (WGPULoadOp)1;
 if([s isEqualToString:@"clear"])return (WGPULoadOp)2;
 if([s isEqualToString:@"expand-resolve-texture"])return (WGPULoadOp)3;
 return (WGPULoadOp)0; }
static WGPUMipmapFilterMode enumMipmapFilterMode(JSValue *v) { if(v.isNumber)return (WGPUMipmapFilterMode)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUMipmapFilterMode)0;
 if([s isEqualToString:@"nearest"])return (WGPUMipmapFilterMode)1;
 if([s isEqualToString:@"linear"])return (WGPUMipmapFilterMode)2;
 return (WGPUMipmapFilterMode)0; }
static WGPUOptionalBool enumOptionalBool(JSValue *v) { if(v.isNumber)return (WGPUOptionalBool)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"false"])return (WGPUOptionalBool)0;
 if([s isEqualToString:@"true"])return (WGPUOptionalBool)1;
 if([s isEqualToString:@"undefined"])return (WGPUOptionalBool)2;
 return (WGPUOptionalBool)0; }
static WGPUPrimitiveTopology enumPrimitiveTopology(JSValue *v) { if(v.isNumber)return (WGPUPrimitiveTopology)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUPrimitiveTopology)0;
 if([s isEqualToString:@"point-list"])return (WGPUPrimitiveTopology)1;
 if([s isEqualToString:@"line-list"])return (WGPUPrimitiveTopology)2;
 if([s isEqualToString:@"line-strip"])return (WGPUPrimitiveTopology)3;
 if([s isEqualToString:@"triangle-list"])return (WGPUPrimitiveTopology)4;
 if([s isEqualToString:@"triangle-strip"])return (WGPUPrimitiveTopology)5;
 return (WGPUPrimitiveTopology)0; }
static WGPUQueryType enumQueryType(JSValue *v) { if(v.isNumber)return (WGPUQueryType)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"occlusion"])return (WGPUQueryType)1;
 if([s isEqualToString:@"timestamp"])return (WGPUQueryType)2;
 return (WGPUQueryType)0; }
static WGPUSamplerBindingType enumSamplerBindingType(JSValue *v) { if(v.isNumber)return (WGPUSamplerBindingType)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUSamplerBindingType)0;
 if([s isEqualToString:@"filtering"])return (WGPUSamplerBindingType)2;
 if([s isEqualToString:@"non-filtering"])return (WGPUSamplerBindingType)3;
 if([s isEqualToString:@"comparison"])return (WGPUSamplerBindingType)4;
 return (WGPUSamplerBindingType)0; }
static WGPUStencilOperation enumStencilOperation(JSValue *v) { if(v.isNumber)return (WGPUStencilOperation)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUStencilOperation)0;
 if([s isEqualToString:@"keep"])return (WGPUStencilOperation)1;
 if([s isEqualToString:@"zero"])return (WGPUStencilOperation)2;
 if([s isEqualToString:@"replace"])return (WGPUStencilOperation)3;
 if([s isEqualToString:@"invert"])return (WGPUStencilOperation)4;
 if([s isEqualToString:@"increment-clamp"])return (WGPUStencilOperation)5;
 if([s isEqualToString:@"decrement-clamp"])return (WGPUStencilOperation)6;
 if([s isEqualToString:@"increment-wrap"])return (WGPUStencilOperation)7;
 if([s isEqualToString:@"decrement-wrap"])return (WGPUStencilOperation)8;
 return (WGPUStencilOperation)0; }
static WGPUStorageTextureAccess enumStorageTextureAccess(JSValue *v) { if(v.isNumber)return (WGPUStorageTextureAccess)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUStorageTextureAccess)0;
 if([s isEqualToString:@"write-only"])return (WGPUStorageTextureAccess)2;
 if([s isEqualToString:@"read-only"])return (WGPUStorageTextureAccess)3;
 if([s isEqualToString:@"read-write"])return (WGPUStorageTextureAccess)4;
 return (WGPUStorageTextureAccess)0; }
static WGPUStoreOp enumStoreOp(JSValue *v) { if(v.isNumber)return (WGPUStoreOp)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUStoreOp)0;
 if([s isEqualToString:@"store"])return (WGPUStoreOp)1;
 if([s isEqualToString:@"discard"])return (WGPUStoreOp)2;
 return (WGPUStoreOp)0; }
static WGPUTextureAspect enumTextureAspect(JSValue *v) { if(v.isNumber)return (WGPUTextureAspect)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUTextureAspect)0;
 if([s isEqualToString:@"all"])return (WGPUTextureAspect)1;
 if([s isEqualToString:@"stencil-only"])return (WGPUTextureAspect)2;
 if([s isEqualToString:@"depth-only"])return (WGPUTextureAspect)3;
 if([s isEqualToString:@"plane-0only"])return (WGPUTextureAspect)0;
 if([s isEqualToString:@"plane-1only"])return (WGPUTextureAspect)1;
 if([s isEqualToString:@"plane-2only"])return (WGPUTextureAspect)2;
 return (WGPUTextureAspect)0; }
static WGPUTextureDimension enumTextureDimension(JSValue *v) { if(v.isNumber)return (WGPUTextureDimension)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUTextureDimension)0;
 if([s isEqualToString:@"1d"])return (WGPUTextureDimension)1;
 if([s isEqualToString:@"2d"])return (WGPUTextureDimension)2;
 if([s isEqualToString:@"3d"])return (WGPUTextureDimension)3;
 return (WGPUTextureDimension)0; }
static WGPUTextureFormat enumTextureFormat(JSValue *v) { if(v.isNumber)return (WGPUTextureFormat)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"r8unorm"])return (WGPUTextureFormat)1;
 if([s isEqualToString:@"r8snorm"])return (WGPUTextureFormat)2;
 if([s isEqualToString:@"r8uint"])return (WGPUTextureFormat)3;
 if([s isEqualToString:@"r8sint"])return (WGPUTextureFormat)4;
 if([s isEqualToString:@"r16unorm"])return (WGPUTextureFormat)5;
 if([s isEqualToString:@"r16snorm"])return (WGPUTextureFormat)6;
 if([s isEqualToString:@"r16uint"])return (WGPUTextureFormat)7;
 if([s isEqualToString:@"r16sint"])return (WGPUTextureFormat)8;
 if([s isEqualToString:@"r16float"])return (WGPUTextureFormat)9;
 if([s isEqualToString:@"rg8unorm"])return (WGPUTextureFormat)10;
 if([s isEqualToString:@"rg8snorm"])return (WGPUTextureFormat)11;
 if([s isEqualToString:@"rg8uint"])return (WGPUTextureFormat)12;
 if([s isEqualToString:@"rg8sint"])return (WGPUTextureFormat)13;
 if([s isEqualToString:@"r32float"])return (WGPUTextureFormat)14;
 if([s isEqualToString:@"r32uint"])return (WGPUTextureFormat)15;
 if([s isEqualToString:@"r32sint"])return (WGPUTextureFormat)16;
 if([s isEqualToString:@"rg16unorm"])return (WGPUTextureFormat)17;
 if([s isEqualToString:@"rg16snorm"])return (WGPUTextureFormat)18;
 if([s isEqualToString:@"rg16uint"])return (WGPUTextureFormat)19;
 if([s isEqualToString:@"rg16sint"])return (WGPUTextureFormat)20;
 if([s isEqualToString:@"rg16float"])return (WGPUTextureFormat)21;
 if([s isEqualToString:@"rgba8unorm"])return (WGPUTextureFormat)22;
 if([s isEqualToString:@"rgba8unorm-srgb"])return (WGPUTextureFormat)23;
 if([s isEqualToString:@"rgba8snorm"])return (WGPUTextureFormat)24;
 if([s isEqualToString:@"rgba8uint"])return (WGPUTextureFormat)25;
 if([s isEqualToString:@"rgba8sint"])return (WGPUTextureFormat)26;
 if([s isEqualToString:@"bgra8unorm"])return (WGPUTextureFormat)27;
 if([s isEqualToString:@"bgra8unorm-srgb"])return (WGPUTextureFormat)28;
 if([s isEqualToString:@"rgb10a2uint"])return (WGPUTextureFormat)29;
 if([s isEqualToString:@"rgb10a2unorm"])return (WGPUTextureFormat)30;
 if([s isEqualToString:@"rg11b10ufloat"])return (WGPUTextureFormat)31;
 if([s isEqualToString:@"rgb9e5ufloat"])return (WGPUTextureFormat)32;
 if([s isEqualToString:@"rg32float"])return (WGPUTextureFormat)33;
 if([s isEqualToString:@"rg32uint"])return (WGPUTextureFormat)34;
 if([s isEqualToString:@"rg32sint"])return (WGPUTextureFormat)35;
 if([s isEqualToString:@"rgba16unorm"])return (WGPUTextureFormat)36;
 if([s isEqualToString:@"rgba16snorm"])return (WGPUTextureFormat)37;
 if([s isEqualToString:@"rgba16uint"])return (WGPUTextureFormat)38;
 if([s isEqualToString:@"rgba16sint"])return (WGPUTextureFormat)39;
 if([s isEqualToString:@"rgba16float"])return (WGPUTextureFormat)40;
 if([s isEqualToString:@"rgba32float"])return (WGPUTextureFormat)41;
 if([s isEqualToString:@"rgba32uint"])return (WGPUTextureFormat)42;
 if([s isEqualToString:@"rgba32sint"])return (WGPUTextureFormat)43;
 if([s isEqualToString:@"stencil8"])return (WGPUTextureFormat)44;
 if([s isEqualToString:@"depth16unorm"])return (WGPUTextureFormat)45;
 if([s isEqualToString:@"depth24plus"])return (WGPUTextureFormat)46;
 if([s isEqualToString:@"depth24plus-stencil8"])return (WGPUTextureFormat)47;
 if([s isEqualToString:@"depth32float"])return (WGPUTextureFormat)48;
 if([s isEqualToString:@"depth32float-stencil8"])return (WGPUTextureFormat)49;
 if([s isEqualToString:@"bc1-rgba-unorm"])return (WGPUTextureFormat)50;
 if([s isEqualToString:@"bc1-rgba-unorm-srgb"])return (WGPUTextureFormat)51;
 if([s isEqualToString:@"bc2-rgba-unorm"])return (WGPUTextureFormat)52;
 if([s isEqualToString:@"bc2-rgba-unorm-srgb"])return (WGPUTextureFormat)53;
 if([s isEqualToString:@"bc3-rgba-unorm"])return (WGPUTextureFormat)54;
 if([s isEqualToString:@"bc3-rgba-unorm-srgb"])return (WGPUTextureFormat)55;
 if([s isEqualToString:@"bc4-r-unorm"])return (WGPUTextureFormat)56;
 if([s isEqualToString:@"bc4-r-snorm"])return (WGPUTextureFormat)57;
 if([s isEqualToString:@"bc5-rg-unorm"])return (WGPUTextureFormat)58;
 if([s isEqualToString:@"bc5-rg-snorm"])return (WGPUTextureFormat)59;
 if([s isEqualToString:@"bc6h-rgb-ufloat"])return (WGPUTextureFormat)60;
 if([s isEqualToString:@"bc6h-rgb-float"])return (WGPUTextureFormat)61;
 if([s isEqualToString:@"bc7-rgba-unorm"])return (WGPUTextureFormat)62;
 if([s isEqualToString:@"bc7-rgba-unorm-srgb"])return (WGPUTextureFormat)63;
 if([s isEqualToString:@"etc2-rgb8unorm"])return (WGPUTextureFormat)64;
 if([s isEqualToString:@"etc2-rgb8unorm-srgb"])return (WGPUTextureFormat)65;
 if([s isEqualToString:@"etc2-rgb8a1unorm"])return (WGPUTextureFormat)66;
 if([s isEqualToString:@"etc2-rgb8a1unorm-srgb"])return (WGPUTextureFormat)67;
 if([s isEqualToString:@"etc2-rgba8unorm"])return (WGPUTextureFormat)68;
 if([s isEqualToString:@"etc2-rgba8unorm-srgb"])return (WGPUTextureFormat)69;
 if([s isEqualToString:@"eac-r11unorm"])return (WGPUTextureFormat)70;
 if([s isEqualToString:@"eac-r11snorm"])return (WGPUTextureFormat)71;
 if([s isEqualToString:@"eac-rg11unorm"])return (WGPUTextureFormat)72;
 if([s isEqualToString:@"eac-rg11snorm"])return (WGPUTextureFormat)73;
 if([s isEqualToString:@"astc-4x4-unorm"])return (WGPUTextureFormat)74;
 if([s isEqualToString:@"astc-4x4-unorm-srgb"])return (WGPUTextureFormat)75;
 if([s isEqualToString:@"astc-5x4-unorm"])return (WGPUTextureFormat)76;
 if([s isEqualToString:@"astc-5x4-unorm-srgb"])return (WGPUTextureFormat)77;
 if([s isEqualToString:@"astc-5x5-unorm"])return (WGPUTextureFormat)78;
 if([s isEqualToString:@"astc-5x5-unorm-srgb"])return (WGPUTextureFormat)79;
 if([s isEqualToString:@"astc-6x5-unorm"])return (WGPUTextureFormat)80;
 if([s isEqualToString:@"astc-6x5-unorm-srgb"])return (WGPUTextureFormat)81;
 if([s isEqualToString:@"astc-6x6-unorm"])return (WGPUTextureFormat)82;
 if([s isEqualToString:@"astc-6x6-unorm-srgb"])return (WGPUTextureFormat)83;
 if([s isEqualToString:@"astc-8x5-unorm"])return (WGPUTextureFormat)84;
 if([s isEqualToString:@"astc-8x5-unorm-srgb"])return (WGPUTextureFormat)85;
 if([s isEqualToString:@"astc-8x6-unorm"])return (WGPUTextureFormat)86;
 if([s isEqualToString:@"astc-8x6-unorm-srgb"])return (WGPUTextureFormat)87;
 if([s isEqualToString:@"astc-8x8-unorm"])return (WGPUTextureFormat)88;
 if([s isEqualToString:@"astc-8x8-unorm-srgb"])return (WGPUTextureFormat)89;
 if([s isEqualToString:@"astc-10x5-unorm"])return (WGPUTextureFormat)90;
 if([s isEqualToString:@"astc-10x5-unorm-srgb"])return (WGPUTextureFormat)91;
 if([s isEqualToString:@"astc-10x6-unorm"])return (WGPUTextureFormat)92;
 if([s isEqualToString:@"astc-10x6-unorm-srgb"])return (WGPUTextureFormat)93;
 if([s isEqualToString:@"astc-10x8-unorm"])return (WGPUTextureFormat)94;
 if([s isEqualToString:@"astc-10x8-unorm-srgb"])return (WGPUTextureFormat)95;
 if([s isEqualToString:@"astc-10x10-unorm"])return (WGPUTextureFormat)96;
 if([s isEqualToString:@"astc-10x10-unorm-srgb"])return (WGPUTextureFormat)97;
 if([s isEqualToString:@"astc-12x10-unorm"])return (WGPUTextureFormat)98;
 if([s isEqualToString:@"astc-12x10-unorm-srgb"])return (WGPUTextureFormat)99;
 if([s isEqualToString:@"astc-12x12-unorm"])return (WGPUTextureFormat)100;
 if([s isEqualToString:@"astc-12x12-unorm-srgb"])return (WGPUTextureFormat)101;
 if([s isEqualToString:@"r8bg8biplanar-420unorm"])return (WGPUTextureFormat)0;
 if([s isEqualToString:@"r10x6bg10x6biplanar-420unorm"])return (WGPUTextureFormat)1;
 if([s isEqualToString:@"r8bg8a8triplanar-420unorm"])return (WGPUTextureFormat)2;
 if([s isEqualToString:@"r8bg8biplanar-422unorm"])return (WGPUTextureFormat)3;
 if([s isEqualToString:@"r8bg8biplanar-444unorm"])return (WGPUTextureFormat)4;
 if([s isEqualToString:@"r10x6bg10x6biplanar-422unorm"])return (WGPUTextureFormat)5;
 if([s isEqualToString:@"r10x6bg10x6biplanar-444unorm"])return (WGPUTextureFormat)6;
 if([s isEqualToString:@"opaque-y-cb-cr-android"])return (WGPUTextureFormat)7;
 return (WGPUTextureFormat)0; }
static WGPUTextureSampleType enumTextureSampleType(JSValue *v) { if(v.isNumber)return (WGPUTextureSampleType)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUTextureSampleType)0;
 if([s isEqualToString:@"float"])return (WGPUTextureSampleType)2;
 if([s isEqualToString:@"unfilterable-float"])return (WGPUTextureSampleType)3;
 if([s isEqualToString:@"depth"])return (WGPUTextureSampleType)4;
 if([s isEqualToString:@"sint"])return (WGPUTextureSampleType)5;
 if([s isEqualToString:@"uint"])return (WGPUTextureSampleType)6;
 return (WGPUTextureSampleType)0; }
static WGPUTextureViewDimension enumTextureViewDimension(JSValue *v) { if(v.isNumber)return (WGPUTextureViewDimension)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUTextureViewDimension)0;
 if([s isEqualToString:@"1d"])return (WGPUTextureViewDimension)1;
 if([s isEqualToString:@"2d"])return (WGPUTextureViewDimension)2;
 if([s isEqualToString:@"2d-array"])return (WGPUTextureViewDimension)3;
 if([s isEqualToString:@"cube"])return (WGPUTextureViewDimension)4;
 if([s isEqualToString:@"cube-array"])return (WGPUTextureViewDimension)5;
 if([s isEqualToString:@"3d"])return (WGPUTextureViewDimension)6;
 return (WGPUTextureViewDimension)0; }
static WGPUVertexFormat enumVertexFormat(JSValue *v) { if(v.isNumber)return (WGPUVertexFormat)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"uint8"])return (WGPUVertexFormat)1;
 if([s isEqualToString:@"uint8x2"])return (WGPUVertexFormat)2;
 if([s isEqualToString:@"uint8x4"])return (WGPUVertexFormat)3;
 if([s isEqualToString:@"sint8"])return (WGPUVertexFormat)4;
 if([s isEqualToString:@"sint8x2"])return (WGPUVertexFormat)5;
 if([s isEqualToString:@"sint8x4"])return (WGPUVertexFormat)6;
 if([s isEqualToString:@"unorm8"])return (WGPUVertexFormat)7;
 if([s isEqualToString:@"unorm8x2"])return (WGPUVertexFormat)8;
 if([s isEqualToString:@"unorm8x4"])return (WGPUVertexFormat)9;
 if([s isEqualToString:@"snorm8"])return (WGPUVertexFormat)10;
 if([s isEqualToString:@"snorm8x2"])return (WGPUVertexFormat)11;
 if([s isEqualToString:@"snorm8x4"])return (WGPUVertexFormat)12;
 if([s isEqualToString:@"uint16"])return (WGPUVertexFormat)13;
 if([s isEqualToString:@"uint16x2"])return (WGPUVertexFormat)14;
 if([s isEqualToString:@"uint16x4"])return (WGPUVertexFormat)15;
 if([s isEqualToString:@"sint16"])return (WGPUVertexFormat)16;
 if([s isEqualToString:@"sint16x2"])return (WGPUVertexFormat)17;
 if([s isEqualToString:@"sint16x4"])return (WGPUVertexFormat)18;
 if([s isEqualToString:@"unorm16"])return (WGPUVertexFormat)19;
 if([s isEqualToString:@"unorm16x2"])return (WGPUVertexFormat)20;
 if([s isEqualToString:@"unorm16x4"])return (WGPUVertexFormat)21;
 if([s isEqualToString:@"snorm16"])return (WGPUVertexFormat)22;
 if([s isEqualToString:@"snorm16x2"])return (WGPUVertexFormat)23;
 if([s isEqualToString:@"snorm16x4"])return (WGPUVertexFormat)24;
 if([s isEqualToString:@"float16"])return (WGPUVertexFormat)25;
 if([s isEqualToString:@"float16x2"])return (WGPUVertexFormat)26;
 if([s isEqualToString:@"float16x4"])return (WGPUVertexFormat)27;
 if([s isEqualToString:@"float32"])return (WGPUVertexFormat)28;
 if([s isEqualToString:@"float32x2"])return (WGPUVertexFormat)29;
 if([s isEqualToString:@"float32x3"])return (WGPUVertexFormat)30;
 if([s isEqualToString:@"float32x4"])return (WGPUVertexFormat)31;
 if([s isEqualToString:@"uint32"])return (WGPUVertexFormat)32;
 if([s isEqualToString:@"uint32x2"])return (WGPUVertexFormat)33;
 if([s isEqualToString:@"uint32x3"])return (WGPUVertexFormat)34;
 if([s isEqualToString:@"uint32x4"])return (WGPUVertexFormat)35;
 if([s isEqualToString:@"sint32"])return (WGPUVertexFormat)36;
 if([s isEqualToString:@"sint32x2"])return (WGPUVertexFormat)37;
 if([s isEqualToString:@"sint32x3"])return (WGPUVertexFormat)38;
 if([s isEqualToString:@"sint32x4"])return (WGPUVertexFormat)39;
 if([s isEqualToString:@"unorm10-10-10-2"])return (WGPUVertexFormat)40;
 if([s isEqualToString:@"unorm8x4-bgra"])return (WGPUVertexFormat)41;
 if([s isEqualToString:@"snorm10-10-10-2"])return (WGPUVertexFormat)42;
 return (WGPUVertexFormat)0; }
static WGPUVertexStepMode enumVertexStepMode(JSValue *v) { if(v.isNumber)return (WGPUVertexStepMode)v.toUInt32;NSString *s=v.toString;
 if([s isEqualToString:@"undefined"])return (WGPUVertexStepMode)0;
 if([s isEqualToString:@"vertex"])return (WGPUVertexStepMode)1;
 if([s isEqualToString:@"instance"])return (WGPUVertexStepMode)2;
 return (WGPUVertexStepMode)0; }
static WGPUBindGroupDescriptor readBindGroupDescriptor(JSValue *v,Arena &a){ WGPUBindGroupDescriptor d=WGPU_BIND_GROUP_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 if(present(v[@"layout"]))d.layout=(WGPUBindGroupLayout)gpuHandle(v[@"layout"]);
 if(present(v[@"entries"])){ JSValue *values=v[@"entries"];size_t n=values[@"length"].toUInt32;auto *p=a.alloc<WGPUBindGroupEntry>(n);for(size_t i=0;i<n;i++)p[i]=readBindGroupEntry([values valueAtIndex:i],a);d.entries=p;d.entryCount=n; }
 return d; }
static WGPUBindGroupEntry readBindGroupEntry(JSValue *v,Arena &a){ WGPUBindGroupEntry d=WGPU_BIND_GROUP_ENTRY_INIT;if(!present(v))return d;
 v=normalizeBinding(v);
 if(present(v[@"binding"]))d.binding=(uint32_t)[v[@"binding"] toDouble];
 if(present(v[@"buffer"]))d.buffer=(WGPUBuffer)gpuHandle(v[@"buffer"]);
 if(present(v[@"offset"]))d.offset=(uint64_t)[v[@"offset"] toDouble];
 if(present(v[@"size"]))d.size=(uint64_t)[v[@"size"] toDouble];
 if(present(v[@"sampler"]))d.sampler=(WGPUSampler)gpuHandle(v[@"sampler"]);
 if(present(v[@"textureView"]))d.textureView=(WGPUTextureView)gpuHandle(v[@"textureView"]);
 return d; }
static WGPUBindGroupLayoutDescriptor readBindGroupLayoutDescriptor(JSValue *v,Arena &a){ WGPUBindGroupLayoutDescriptor d=WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 if(present(v[@"entries"])){ JSValue *values=v[@"entries"];size_t n=values[@"length"].toUInt32;auto *p=a.alloc<WGPUBindGroupLayoutEntry>(n);for(size_t i=0;i<n;i++)p[i]=readBindGroupLayoutEntry([values valueAtIndex:i],a);d.entries=p;d.entryCount=n; }
 return d; }
static WGPUBindGroupLayoutEntry readBindGroupLayoutEntry(JSValue *v,Arena &a){ WGPUBindGroupLayoutEntry d=WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT;if(!present(v))return d;
 if(present(v[@"binding"]))d.binding=(uint32_t)[v[@"binding"] toDouble];
 if(present(v[@"visibility"]))d.visibility=(WGPUShaderStage)[v[@"visibility"] toDouble];
 if(present(v[@"bindingArraySize"]))d.bindingArraySize=(uint32_t)[v[@"bindingArraySize"] toDouble];
 if(present(v[@"buffer"]))d.buffer=readBufferBindingLayout(v[@"buffer"],a);
 if(present(v[@"sampler"]))d.sampler=readSamplerBindingLayout(v[@"sampler"],a);
 if(present(v[@"texture"]))d.texture=readTextureBindingLayout(v[@"texture"],a);
 if(present(v[@"storageTexture"]))d.storageTexture=readStorageTextureBindingLayout(v[@"storageTexture"],a);
 return d; }
static WGPUBlendComponent readBlendComponent(JSValue *v,Arena &a){ WGPUBlendComponent d=WGPU_BLEND_COMPONENT_INIT;if(!present(v))return d;
 if(present(v[@"operation"]))d.operation=enumBlendOperation(v[@"operation"]);
 if(present(v[@"srcFactor"]))d.srcFactor=enumBlendFactor(v[@"srcFactor"]);
 if(present(v[@"dstFactor"]))d.dstFactor=enumBlendFactor(v[@"dstFactor"]);
 return d; }
static WGPUBlendState readBlendState(JSValue *v,Arena &a){ WGPUBlendState d=WGPU_BLEND_STATE_INIT;if(!present(v))return d;
 if(present(v[@"color"]))d.color=readBlendComponent(v[@"color"],a);
 if(present(v[@"alpha"]))d.alpha=readBlendComponent(v[@"alpha"],a);
 return d; }
static WGPUBufferBindingLayout readBufferBindingLayout(JSValue *v,Arena &a){ WGPUBufferBindingLayout d=WGPU_BUFFER_BINDING_LAYOUT_INIT;if(!present(v))return d;
 if(present(v[@"type"]))d.type=enumBufferBindingType(v[@"type"]);
 if(present(v[@"hasDynamicOffset"]))d.hasDynamicOffset=(bool)[v[@"hasDynamicOffset"] toDouble];
 if(present(v[@"minBindingSize"]))d.minBindingSize=(uint64_t)[v[@"minBindingSize"] toDouble];
 return d; }
static WGPUBufferDescriptor readBufferDescriptor(JSValue *v,Arena &a){ WGPUBufferDescriptor d=WGPU_BUFFER_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 if(present(v[@"usage"]))d.usage=(WGPUBufferUsage)[v[@"usage"] toDouble];
 if(present(v[@"size"]))d.size=(uint64_t)[v[@"size"] toDouble];
 if(present(v[@"mappedAtCreation"]))d.mappedAtCreation=(bool)[v[@"mappedAtCreation"] toDouble];
 return d; }
static WGPUColor readColor(JSValue *v,Arena &a){ WGPUColor d=WGPU_COLOR_INIT;if(!present(v))return d;
 v=normalizeVector(v,@"color");
 if(present(v[@"r"]))d.r=(double)[v[@"r"] toDouble];
 if(present(v[@"g"]))d.g=(double)[v[@"g"] toDouble];
 if(present(v[@"b"]))d.b=(double)[v[@"b"] toDouble];
 if(present(v[@"a"]))d.a=(double)[v[@"a"] toDouble];
 return d; }
static WGPUColorTargetState readColorTargetState(JSValue *v,Arena &a){ WGPUColorTargetState d=WGPU_COLOR_TARGET_STATE_INIT;if(!present(v))return d;
 if(present(v[@"format"]))d.format=enumTextureFormat(v[@"format"]);
 if(present(v[@"blend"])){auto *p=a.alloc<WGPUBlendState>();*p=readBlendState(v[@"blend"],a);d.blend=p;}
 if(present(v[@"writeMask"]))d.writeMask=(WGPUColorWriteMask)[v[@"writeMask"] toDouble];
 return d; }
static WGPUCommandBufferDescriptor readCommandBufferDescriptor(JSValue *v,Arena &a){ WGPUCommandBufferDescriptor d=WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 return d; }
static WGPUCommandEncoderDescriptor readCommandEncoderDescriptor(JSValue *v,Arena &a){ WGPUCommandEncoderDescriptor d=WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 return d; }
static WGPUComputePassDescriptor readComputePassDescriptor(JSValue *v,Arena &a){ WGPUComputePassDescriptor d=WGPU_COMPUTE_PASS_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 if(present(v[@"timestampWrites"])){auto *p=a.alloc<WGPUPassTimestampWrites>();*p=readPassTimestampWrites(v[@"timestampWrites"],a);d.timestampWrites=p;}
 return d; }
static WGPUComputePipelineDescriptor readComputePipelineDescriptor(JSValue *v,Arena &a){ WGPUComputePipelineDescriptor d=WGPU_COMPUTE_PIPELINE_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 if(present(v[@"layout"]))d.layout=(WGPUPipelineLayout)gpuHandle(v[@"layout"]);
 if(present(v[@"compute"]))d.compute=readComputeState(v[@"compute"],a);
 return d; }
static WGPUComputeState readComputeState(JSValue *v,Arena &a){ WGPUComputeState d=WGPU_COMPUTE_STATE_INIT;if(!present(v))return d;
 if(present(v[@"module"]))d.module=(WGPUShaderModule)gpuHandle(v[@"module"]);
 if(present(v[@"entryPoint"]))d.entryPoint=a.string(v[@"entryPoint"]);
 if(present(v[@"constants"])){ JSValue *values=v[@"constants"];size_t n=values[@"length"].toUInt32;auto *p=a.alloc<WGPUConstantEntry>(n);for(size_t i=0;i<n;i++)p[i]=readConstantEntry([values valueAtIndex:i],a);d.constants=p;d.constantCount=n; }
 return d; }
static WGPUConstantEntry readConstantEntry(JSValue *v,Arena &a){ WGPUConstantEntry d=WGPU_CONSTANT_ENTRY_INIT;if(!present(v))return d;
 if(present(v[@"key"]))d.key=a.string(v[@"key"]);
 if(present(v[@"value"]))d.value=(double)[v[@"value"] toDouble];
 return d; }
static WGPUDepthStencilState readDepthStencilState(JSValue *v,Arena &a){ WGPUDepthStencilState d=WGPU_DEPTH_STENCIL_STATE_INIT;if(!present(v))return d;
 if(present(v[@"format"]))d.format=enumTextureFormat(v[@"format"]);
 if(present(v[@"depthWriteEnabled"]))d.depthWriteEnabled=enumOptionalBool(v[@"depthWriteEnabled"]);
 if(present(v[@"depthCompare"]))d.depthCompare=enumCompareFunction(v[@"depthCompare"]);
 if(present(v[@"stencilFront"]))d.stencilFront=readStencilFaceState(v[@"stencilFront"],a);
 if(present(v[@"stencilBack"]))d.stencilBack=readStencilFaceState(v[@"stencilBack"],a);
 if(present(v[@"stencilReadMask"]))d.stencilReadMask=(uint32_t)[v[@"stencilReadMask"] toDouble];
 if(present(v[@"stencilWriteMask"]))d.stencilWriteMask=(uint32_t)[v[@"stencilWriteMask"] toDouble];
 if(present(v[@"depthBias"]))d.depthBias=(int32_t)[v[@"depthBias"] toDouble];
 if(present(v[@"depthBiasSlopeScale"]))d.depthBiasSlopeScale=(float)[v[@"depthBiasSlopeScale"] toDouble];
 if(present(v[@"depthBiasClamp"]))d.depthBiasClamp=(float)[v[@"depthBiasClamp"] toDouble];
 return d; }
static WGPUExtent3D readExtent3D(JSValue *v,Arena &a){ WGPUExtent3D d=WGPU_EXTENT_3D_INIT;if(!present(v))return d;
 v=normalizeVector(v,@"extent 3D");
 if(present(v[@"width"]))d.width=(uint32_t)[v[@"width"] toDouble];
 if(present(v[@"height"]))d.height=(uint32_t)[v[@"height"] toDouble];
 if(present(v[@"depthOrArrayLayers"]))d.depthOrArrayLayers=(uint32_t)[v[@"depthOrArrayLayers"] toDouble];
 return d; }
static WGPUFragmentState readFragmentState(JSValue *v,Arena &a){ WGPUFragmentState d=WGPU_FRAGMENT_STATE_INIT;if(!present(v))return d;
 if(present(v[@"module"]))d.module=(WGPUShaderModule)gpuHandle(v[@"module"]);
 if(present(v[@"entryPoint"]))d.entryPoint=a.string(v[@"entryPoint"]);
 if(present(v[@"constants"])){ JSValue *values=v[@"constants"];size_t n=values[@"length"].toUInt32;auto *p=a.alloc<WGPUConstantEntry>(n);for(size_t i=0;i<n;i++)p[i]=readConstantEntry([values valueAtIndex:i],a);d.constants=p;d.constantCount=n; }
 if(present(v[@"targets"])){ JSValue *values=v[@"targets"];size_t n=values[@"length"].toUInt32;auto *p=a.alloc<WGPUColorTargetState>(n);for(size_t i=0;i<n;i++)p[i]=readColorTargetState([values valueAtIndex:i],a);d.targets=p;d.targetCount=n; }
 return d; }
static WGPULimits readLimits(JSValue *v,Arena &a){ WGPULimits d=WGPU_LIMITS_INIT;if(!present(v))return d;
 if(present(v[@"maxTextureDimension1D"]))d.maxTextureDimension1D=(uint32_t)[v[@"maxTextureDimension1D"] toDouble];
 if(present(v[@"maxTextureDimension2D"]))d.maxTextureDimension2D=(uint32_t)[v[@"maxTextureDimension2D"] toDouble];
 if(present(v[@"maxTextureDimension3D"]))d.maxTextureDimension3D=(uint32_t)[v[@"maxTextureDimension3D"] toDouble];
 if(present(v[@"maxTextureArrayLayers"]))d.maxTextureArrayLayers=(uint32_t)[v[@"maxTextureArrayLayers"] toDouble];
 if(present(v[@"maxBindGroups"]))d.maxBindGroups=(uint32_t)[v[@"maxBindGroups"] toDouble];
 if(present(v[@"maxBindGroupsPlusVertexBuffers"]))d.maxBindGroupsPlusVertexBuffers=(uint32_t)[v[@"maxBindGroupsPlusVertexBuffers"] toDouble];
 if(present(v[@"maxBindingsPerBindGroup"]))d.maxBindingsPerBindGroup=(uint32_t)[v[@"maxBindingsPerBindGroup"] toDouble];
 if(present(v[@"maxDynamicUniformBuffersPerPipelineLayout"]))d.maxDynamicUniformBuffersPerPipelineLayout=(uint32_t)[v[@"maxDynamicUniformBuffersPerPipelineLayout"] toDouble];
 if(present(v[@"maxDynamicStorageBuffersPerPipelineLayout"]))d.maxDynamicStorageBuffersPerPipelineLayout=(uint32_t)[v[@"maxDynamicStorageBuffersPerPipelineLayout"] toDouble];
 if(present(v[@"maxSampledTexturesPerShaderStage"]))d.maxSampledTexturesPerShaderStage=(uint32_t)[v[@"maxSampledTexturesPerShaderStage"] toDouble];
 if(present(v[@"maxSamplersPerShaderStage"]))d.maxSamplersPerShaderStage=(uint32_t)[v[@"maxSamplersPerShaderStage"] toDouble];
 if(present(v[@"maxStorageBuffersPerShaderStage"]))d.maxStorageBuffersPerShaderStage=(uint32_t)[v[@"maxStorageBuffersPerShaderStage"] toDouble];
 if(present(v[@"maxStorageTexturesPerShaderStage"]))d.maxStorageTexturesPerShaderStage=(uint32_t)[v[@"maxStorageTexturesPerShaderStage"] toDouble];
 if(present(v[@"maxUniformBuffersPerShaderStage"]))d.maxUniformBuffersPerShaderStage=(uint32_t)[v[@"maxUniformBuffersPerShaderStage"] toDouble];
 if(present(v[@"maxUniformBufferBindingSize"]))d.maxUniformBufferBindingSize=(uint64_t)[v[@"maxUniformBufferBindingSize"] toDouble];
 if(present(v[@"maxStorageBufferBindingSize"]))d.maxStorageBufferBindingSize=(uint64_t)[v[@"maxStorageBufferBindingSize"] toDouble];
 if(present(v[@"minUniformBufferOffsetAlignment"]))d.minUniformBufferOffsetAlignment=(uint32_t)[v[@"minUniformBufferOffsetAlignment"] toDouble];
 if(present(v[@"minStorageBufferOffsetAlignment"]))d.minStorageBufferOffsetAlignment=(uint32_t)[v[@"minStorageBufferOffsetAlignment"] toDouble];
 if(present(v[@"maxVertexBuffers"]))d.maxVertexBuffers=(uint32_t)[v[@"maxVertexBuffers"] toDouble];
 if(present(v[@"maxBufferSize"]))d.maxBufferSize=(uint64_t)[v[@"maxBufferSize"] toDouble];
 if(present(v[@"maxVertexAttributes"]))d.maxVertexAttributes=(uint32_t)[v[@"maxVertexAttributes"] toDouble];
 if(present(v[@"maxVertexBufferArrayStride"]))d.maxVertexBufferArrayStride=(uint32_t)[v[@"maxVertexBufferArrayStride"] toDouble];
 if(present(v[@"maxInterStageShaderVariables"]))d.maxInterStageShaderVariables=(uint32_t)[v[@"maxInterStageShaderVariables"] toDouble];
 if(present(v[@"maxColorAttachments"]))d.maxColorAttachments=(uint32_t)[v[@"maxColorAttachments"] toDouble];
 if(present(v[@"maxColorAttachmentBytesPerSample"]))d.maxColorAttachmentBytesPerSample=(uint32_t)[v[@"maxColorAttachmentBytesPerSample"] toDouble];
 if(present(v[@"maxComputeWorkgroupStorageSize"]))d.maxComputeWorkgroupStorageSize=(uint32_t)[v[@"maxComputeWorkgroupStorageSize"] toDouble];
 if(present(v[@"maxComputeInvocationsPerWorkgroup"]))d.maxComputeInvocationsPerWorkgroup=(uint32_t)[v[@"maxComputeInvocationsPerWorkgroup"] toDouble];
 if(present(v[@"maxComputeWorkgroupSizeX"]))d.maxComputeWorkgroupSizeX=(uint32_t)[v[@"maxComputeWorkgroupSizeX"] toDouble];
 if(present(v[@"maxComputeWorkgroupSizeY"]))d.maxComputeWorkgroupSizeY=(uint32_t)[v[@"maxComputeWorkgroupSizeY"] toDouble];
 if(present(v[@"maxComputeWorkgroupSizeZ"]))d.maxComputeWorkgroupSizeZ=(uint32_t)[v[@"maxComputeWorkgroupSizeZ"] toDouble];
 if(present(v[@"maxComputeWorkgroupsPerDimension"]))d.maxComputeWorkgroupsPerDimension=(uint32_t)[v[@"maxComputeWorkgroupsPerDimension"] toDouble];
 if(present(v[@"maxImmediateSize"]))d.maxImmediateSize=(uint32_t)[v[@"maxImmediateSize"] toDouble];
 return d; }
static WGPUMultisampleState readMultisampleState(JSValue *v,Arena &a){ WGPUMultisampleState d=WGPU_MULTISAMPLE_STATE_INIT;if(!present(v))return d;
 if(present(v[@"count"]))d.count=(uint32_t)[v[@"count"] toDouble];
 if(present(v[@"mask"]))d.mask=(uint32_t)[v[@"mask"] toDouble];
 if(present(v[@"alphaToCoverageEnabled"]))d.alphaToCoverageEnabled=(bool)[v[@"alphaToCoverageEnabled"] toDouble];
 return d; }
static WGPUOrigin3D readOrigin3D(JSValue *v,Arena &a){ WGPUOrigin3D d=WGPU_ORIGIN_3D_INIT;if(!present(v))return d;
 v=normalizeVector(v,@"origin 3D");
 if(present(v[@"x"]))d.x=(uint32_t)[v[@"x"] toDouble];
 if(present(v[@"y"]))d.y=(uint32_t)[v[@"y"] toDouble];
 if(present(v[@"z"]))d.z=(uint32_t)[v[@"z"] toDouble];
 return d; }
static WGPUPassTimestampWrites readPassTimestampWrites(JSValue *v,Arena &a){ WGPUPassTimestampWrites d=WGPU_PASS_TIMESTAMP_WRITES_INIT;if(!present(v))return d;
 if(present(v[@"querySet"]))d.querySet=(WGPUQuerySet)gpuHandle(v[@"querySet"]);
 if(present(v[@"beginningOfPassWriteIndex"]))d.beginningOfPassWriteIndex=(uint32_t)[v[@"beginningOfPassWriteIndex"] toDouble];
 if(present(v[@"endOfPassWriteIndex"]))d.endOfPassWriteIndex=(uint32_t)[v[@"endOfPassWriteIndex"] toDouble];
 return d; }
static WGPUPipelineLayoutDescriptor readPipelineLayoutDescriptor(JSValue *v,Arena &a){ WGPUPipelineLayoutDescriptor d=WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 if(present(v[@"bindGroupLayouts"])){ JSValue *values=v[@"bindGroupLayouts"];size_t n=values[@"length"].toUInt32;auto *p=a.alloc<WGPUBindGroupLayout>(n);for(size_t i=0;i<n;i++)p[i]=(WGPUBindGroupLayout)gpuHandle([values valueAtIndex:i]);d.bindGroupLayouts=p;d.bindGroupLayoutCount=n; }
 if(present(v[@"immediateSize"]))d.immediateSize=(uint32_t)[v[@"immediateSize"] toDouble];
 return d; }
static WGPUPrimitiveState readPrimitiveState(JSValue *v,Arena &a){ WGPUPrimitiveState d=WGPU_PRIMITIVE_STATE_INIT;if(!present(v))return d;
 if(present(v[@"topology"]))d.topology=enumPrimitiveTopology(v[@"topology"]);
 if(present(v[@"stripIndexFormat"]))d.stripIndexFormat=enumIndexFormat(v[@"stripIndexFormat"]);
 if(present(v[@"frontFace"]))d.frontFace=enumFrontFace(v[@"frontFace"]);
 if(present(v[@"cullMode"]))d.cullMode=enumCullMode(v[@"cullMode"]);
 if(present(v[@"unclippedDepth"]))d.unclippedDepth=(bool)[v[@"unclippedDepth"] toDouble];
 return d; }
static WGPUQuerySetDescriptor readQuerySetDescriptor(JSValue *v,Arena &a){ WGPUQuerySetDescriptor d=WGPU_QUERY_SET_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 if(present(v[@"type"]))d.type=enumQueryType(v[@"type"]);
 if(present(v[@"count"]))d.count=(uint32_t)[v[@"count"] toDouble];
 return d; }
static WGPURenderPassColorAttachment readRenderPassColorAttachment(JSValue *v,Arena &a){ WGPURenderPassColorAttachment d=WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT;if(!present(v))return d;
 if(present(v[@"view"]))d.view=(WGPUTextureView)gpuHandle(v[@"view"]);
 if(present(v[@"depthSlice"]))d.depthSlice=(uint32_t)[v[@"depthSlice"] toDouble];
 if(present(v[@"resolveTarget"]))d.resolveTarget=(WGPUTextureView)gpuHandle(v[@"resolveTarget"]);
 if(present(v[@"loadOp"]))d.loadOp=enumLoadOp(v[@"loadOp"]);
 if(present(v[@"storeOp"]))d.storeOp=enumStoreOp(v[@"storeOp"]);
 if(present(v[@"clearValue"]))d.clearValue=readColor(v[@"clearValue"],a);
 return d; }
static WGPURenderPassDepthStencilAttachment readRenderPassDepthStencilAttachment(JSValue *v,Arena &a){ WGPURenderPassDepthStencilAttachment d=WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT;if(!present(v))return d;
 if(present(v[@"view"]))d.view=(WGPUTextureView)gpuHandle(v[@"view"]);
 if(present(v[@"depthLoadOp"]))d.depthLoadOp=enumLoadOp(v[@"depthLoadOp"]);
 if(present(v[@"depthStoreOp"]))d.depthStoreOp=enumStoreOp(v[@"depthStoreOp"]);
 if(present(v[@"depthClearValue"]))d.depthClearValue=(float)[v[@"depthClearValue"] toDouble];
 if(present(v[@"depthReadOnly"]))d.depthReadOnly=(bool)[v[@"depthReadOnly"] toDouble];
 if(present(v[@"stencilLoadOp"]))d.stencilLoadOp=enumLoadOp(v[@"stencilLoadOp"]);
 if(present(v[@"stencilStoreOp"]))d.stencilStoreOp=enumStoreOp(v[@"stencilStoreOp"]);
 if(present(v[@"stencilClearValue"]))d.stencilClearValue=(uint32_t)[v[@"stencilClearValue"] toDouble];
 if(present(v[@"stencilReadOnly"]))d.stencilReadOnly=(bool)[v[@"stencilReadOnly"] toDouble];
 return d; }
static WGPURenderPassDescriptor readRenderPassDescriptor(JSValue *v,Arena &a){ WGPURenderPassDescriptor d=WGPU_RENDER_PASS_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 if(present(v[@"colorAttachments"])){ JSValue *values=v[@"colorAttachments"];size_t n=values[@"length"].toUInt32;auto *p=a.alloc<WGPURenderPassColorAttachment>(n);for(size_t i=0;i<n;i++)p[i]=readRenderPassColorAttachment([values valueAtIndex:i],a);d.colorAttachments=p;d.colorAttachmentCount=n; }
 if(present(v[@"depthStencilAttachment"])){auto *p=a.alloc<WGPURenderPassDepthStencilAttachment>();*p=readRenderPassDepthStencilAttachment(v[@"depthStencilAttachment"],a);d.depthStencilAttachment=p;}
 if(present(v[@"occlusionQuerySet"]))d.occlusionQuerySet=(WGPUQuerySet)gpuHandle(v[@"occlusionQuerySet"]);
 if(present(v[@"timestampWrites"])){auto *p=a.alloc<WGPUPassTimestampWrites>();*p=readPassTimestampWrites(v[@"timestampWrites"],a);d.timestampWrites=p;}
 return d; }
static WGPURenderPipelineDescriptor readRenderPipelineDescriptor(JSValue *v,Arena &a){ WGPURenderPipelineDescriptor d=WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 if(present(v[@"layout"]))d.layout=(WGPUPipelineLayout)gpuHandle(v[@"layout"]);
 if(present(v[@"vertex"]))d.vertex=readVertexState(v[@"vertex"],a);
 if(present(v[@"primitive"]))d.primitive=readPrimitiveState(v[@"primitive"],a);
 if(present(v[@"depthStencil"])){auto *p=a.alloc<WGPUDepthStencilState>();*p=readDepthStencilState(v[@"depthStencil"],a);d.depthStencil=p;}
 if(present(v[@"multisample"]))d.multisample=readMultisampleState(v[@"multisample"],a);
 if(present(v[@"fragment"])){auto *p=a.alloc<WGPUFragmentState>();*p=readFragmentState(v[@"fragment"],a);d.fragment=p;}
 return d; }
static WGPUSamplerBindingLayout readSamplerBindingLayout(JSValue *v,Arena &a){ WGPUSamplerBindingLayout d=WGPU_SAMPLER_BINDING_LAYOUT_INIT;if(!present(v))return d;
 if(present(v[@"type"]))d.type=enumSamplerBindingType(v[@"type"]);
 return d; }
static WGPUSamplerDescriptor readSamplerDescriptor(JSValue *v,Arena &a){ WGPUSamplerDescriptor d=WGPU_SAMPLER_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 if(present(v[@"addressModeU"]))d.addressModeU=enumAddressMode(v[@"addressModeU"]);
 if(present(v[@"addressModeV"]))d.addressModeV=enumAddressMode(v[@"addressModeV"]);
 if(present(v[@"addressModeW"]))d.addressModeW=enumAddressMode(v[@"addressModeW"]);
 if(present(v[@"magFilter"]))d.magFilter=enumFilterMode(v[@"magFilter"]);
 if(present(v[@"minFilter"]))d.minFilter=enumFilterMode(v[@"minFilter"]);
 if(present(v[@"mipmapFilter"]))d.mipmapFilter=enumMipmapFilterMode(v[@"mipmapFilter"]);
 if(present(v[@"lodMinClamp"]))d.lodMinClamp=(float)[v[@"lodMinClamp"] toDouble];
 if(present(v[@"lodMaxClamp"]))d.lodMaxClamp=(float)[v[@"lodMaxClamp"] toDouble];
 if(present(v[@"compare"]))d.compare=enumCompareFunction(v[@"compare"]);
 if(present(v[@"maxAnisotropy"]))d.maxAnisotropy=(uint16_t)[v[@"maxAnisotropy"] toDouble];
 return d; }
static WGPUShaderModuleDescriptor readShaderModuleDescriptor(JSValue *v,Arena &a){ WGPUShaderModuleDescriptor d=WGPU_SHADER_MODULE_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 auto *source=a.alloc<WGPUShaderSourceWGSL>();*source=WGPU_SHADER_SOURCE_WGSL_INIT;source->code=a.string(v[@"code"]);d.nextInChain=&source->chain;
 return d; }
static WGPUStencilFaceState readStencilFaceState(JSValue *v,Arena &a){ WGPUStencilFaceState d=WGPU_STENCIL_FACE_STATE_INIT;if(!present(v))return d;
 if(present(v[@"compare"]))d.compare=enumCompareFunction(v[@"compare"]);
 if(present(v[@"failOp"]))d.failOp=enumStencilOperation(v[@"failOp"]);
 if(present(v[@"depthFailOp"]))d.depthFailOp=enumStencilOperation(v[@"depthFailOp"]);
 if(present(v[@"passOp"]))d.passOp=enumStencilOperation(v[@"passOp"]);
 return d; }
static WGPUStorageTextureBindingLayout readStorageTextureBindingLayout(JSValue *v,Arena &a){ WGPUStorageTextureBindingLayout d=WGPU_STORAGE_TEXTURE_BINDING_LAYOUT_INIT;if(!present(v))return d;
 if(present(v[@"access"]))d.access=enumStorageTextureAccess(v[@"access"]);
 if(present(v[@"format"]))d.format=enumTextureFormat(v[@"format"]);
 if(present(v[@"viewDimension"]))d.viewDimension=enumTextureViewDimension(v[@"viewDimension"]);
 return d; }
static WGPUTexelCopyBufferInfo readTexelCopyBufferInfo(JSValue *v,Arena &a){ WGPUTexelCopyBufferInfo d=WGPU_TEXEL_COPY_BUFFER_INFO_INIT;if(!present(v))return d;
 if(present(v[@"layout"]))d.layout=readTexelCopyBufferLayout(v[@"layout"],a);
 if(present(v[@"buffer"]))d.buffer=(WGPUBuffer)gpuHandle(v[@"buffer"]);
 d.layout=readTexelCopyBufferLayout(v,a);
 return d; }
static WGPUTexelCopyBufferLayout readTexelCopyBufferLayout(JSValue *v,Arena &a){ WGPUTexelCopyBufferLayout d=WGPU_TEXEL_COPY_BUFFER_LAYOUT_INIT;if(!present(v))return d;
 if(present(v[@"offset"]))d.offset=(uint64_t)[v[@"offset"] toDouble];
 if(present(v[@"bytesPerRow"]))d.bytesPerRow=(uint32_t)[v[@"bytesPerRow"] toDouble];
 if(present(v[@"rowsPerImage"]))d.rowsPerImage=(uint32_t)[v[@"rowsPerImage"] toDouble];
 return d; }
static WGPUTexelCopyTextureInfo readTexelCopyTextureInfo(JSValue *v,Arena &a){ WGPUTexelCopyTextureInfo d=WGPU_TEXEL_COPY_TEXTURE_INFO_INIT;if(!present(v))return d;
 if(present(v[@"texture"]))d.texture=(WGPUTexture)gpuHandle(v[@"texture"]);
 if(present(v[@"mipLevel"]))d.mipLevel=(uint32_t)[v[@"mipLevel"] toDouble];
 if(present(v[@"origin"]))d.origin=readOrigin3D(v[@"origin"],a);
 if(present(v[@"aspect"]))d.aspect=enumTextureAspect(v[@"aspect"]);
 return d; }
static WGPUTextureBindingLayout readTextureBindingLayout(JSValue *v,Arena &a){ WGPUTextureBindingLayout d=WGPU_TEXTURE_BINDING_LAYOUT_INIT;if(!present(v))return d;
 if(present(v[@"sampleType"]))d.sampleType=enumTextureSampleType(v[@"sampleType"]);
 if(present(v[@"viewDimension"]))d.viewDimension=enumTextureViewDimension(v[@"viewDimension"]);
 if(present(v[@"multisampled"]))d.multisampled=(bool)[v[@"multisampled"] toDouble];
 return d; }
static WGPUTextureDescriptor readTextureDescriptor(JSValue *v,Arena &a){ WGPUTextureDescriptor d=WGPU_TEXTURE_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 if(present(v[@"usage"]))d.usage=(WGPUTextureUsage)[v[@"usage"] toDouble];
 if(present(v[@"dimension"]))d.dimension=enumTextureDimension(v[@"dimension"]);
 if(present(v[@"size"]))d.size=readExtent3D(v[@"size"],a);
 if(present(v[@"format"]))d.format=enumTextureFormat(v[@"format"]);
 if(present(v[@"mipLevelCount"]))d.mipLevelCount=(uint32_t)[v[@"mipLevelCount"] toDouble];
 if(present(v[@"sampleCount"]))d.sampleCount=(uint32_t)[v[@"sampleCount"] toDouble];
 if(present(v[@"viewFormats"])){ JSValue *values=v[@"viewFormats"];size_t n=values[@"length"].toUInt32;auto *p=a.alloc<WGPUTextureFormat>(n);for(size_t i=0;i<n;i++)p[i]=enumTextureFormat([values valueAtIndex:i]);d.viewFormats=p;d.viewFormatCount=n; }
 return d; }
static WGPUTextureViewDescriptor readTextureViewDescriptor(JSValue *v,Arena &a){ WGPUTextureViewDescriptor d=WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT;if(!present(v))return d;
 if(present(v[@"label"]))d.label=a.string(v[@"label"]);
 if(present(v[@"format"]))d.format=enumTextureFormat(v[@"format"]);
 if(present(v[@"dimension"]))d.dimension=enumTextureViewDimension(v[@"dimension"]);
 if(present(v[@"baseMipLevel"]))d.baseMipLevel=(uint32_t)[v[@"baseMipLevel"] toDouble];
 if(present(v[@"mipLevelCount"]))d.mipLevelCount=(uint32_t)[v[@"mipLevelCount"] toDouble];
 if(present(v[@"baseArrayLayer"]))d.baseArrayLayer=(uint32_t)[v[@"baseArrayLayer"] toDouble];
 if(present(v[@"arrayLayerCount"]))d.arrayLayerCount=(uint32_t)[v[@"arrayLayerCount"] toDouble];
 if(present(v[@"aspect"]))d.aspect=enumTextureAspect(v[@"aspect"]);
 if(present(v[@"usage"]))d.usage=(WGPUTextureUsage)[v[@"usage"] toDouble];
 return d; }
static WGPUVertexAttribute readVertexAttribute(JSValue *v,Arena &a){ WGPUVertexAttribute d=WGPU_VERTEX_ATTRIBUTE_INIT;if(!present(v))return d;
 if(present(v[@"format"]))d.format=enumVertexFormat(v[@"format"]);
 if(present(v[@"offset"]))d.offset=(uint64_t)[v[@"offset"] toDouble];
 if(present(v[@"shaderLocation"]))d.shaderLocation=(uint32_t)[v[@"shaderLocation"] toDouble];
 return d; }
static WGPUVertexBufferLayout readVertexBufferLayout(JSValue *v,Arena &a){ WGPUVertexBufferLayout d=WGPU_VERTEX_BUFFER_LAYOUT_INIT;if(!present(v))return d;
 if(present(v[@"stepMode"]))d.stepMode=enumVertexStepMode(v[@"stepMode"]);
 if(present(v[@"arrayStride"]))d.arrayStride=(uint64_t)[v[@"arrayStride"] toDouble];
 if(present(v[@"attributes"])){ JSValue *values=v[@"attributes"];size_t n=values[@"length"].toUInt32;auto *p=a.alloc<WGPUVertexAttribute>(n);for(size_t i=0;i<n;i++)p[i]=readVertexAttribute([values valueAtIndex:i],a);d.attributes=p;d.attributeCount=n; }
 return d; }
static WGPUVertexState readVertexState(JSValue *v,Arena &a){ WGPUVertexState d=WGPU_VERTEX_STATE_INIT;if(!present(v))return d;
 if(present(v[@"module"]))d.module=(WGPUShaderModule)gpuHandle(v[@"module"]);
 if(present(v[@"entryPoint"]))d.entryPoint=a.string(v[@"entryPoint"]);
 if(present(v[@"constants"])){ JSValue *values=v[@"constants"];size_t n=values[@"length"].toUInt32;auto *p=a.alloc<WGPUConstantEntry>(n);for(size_t i=0;i<n;i++)p[i]=readConstantEntry([values valueAtIndex:i],a);d.constants=p;d.constantCount=n; }
 if(present(v[@"buffers"])){ JSValue *values=v[@"buffers"];size_t n=values[@"length"].toUInt32;auto *p=a.alloc<WGPUVertexBufferLayout>(n);for(size_t i=0;i<n;i++)p[i]=readVertexBufferLayout([values valueAtIndex:i],a);d.buffers=p;d.bufferCount=n; }
 return d; }
static JSValue *gpuGeneratedCall(NSString *op,JSValue *object,JSValue *args) { Arena a;JSContext *context=args.context;
 if([op isEqualToString:@"device.createBindGroup"]){
 auto p0=readBindGroupDescriptor([args valueAtIndex:0],a);
 auto result=wgpuDeviceCreateBindGroup((WGPUDevice)gpuHandle(object),&p0);return wrapGPU(context,@"bindGroup",result,[](void *p){wgpuBindGroupRelease((WGPUBindGroup)p);});
 }
 if([op isEqualToString:@"device.createBindGroupLayout"]){
 auto p0=readBindGroupLayoutDescriptor([args valueAtIndex:0],a);
 auto result=wgpuDeviceCreateBindGroupLayout((WGPUDevice)gpuHandle(object),&p0);return wrapGPU(context,@"bindGroupLayout",result,[](void *p){wgpuBindGroupLayoutRelease((WGPUBindGroupLayout)p);});
 }
 if([op isEqualToString:@"device.createBuffer"]){
 auto p0=readBufferDescriptor([args valueAtIndex:0],a);
 auto result=wgpuDeviceCreateBuffer((WGPUDevice)gpuHandle(object),&p0);return wrapGPU(context,@"buffer",result,[](void *p){wgpuBufferRelease((WGPUBuffer)p);});
 }
 if([op isEqualToString:@"device.createCommandEncoder"]){
 auto p0=readCommandEncoderDescriptor([args valueAtIndex:0],a);
 auto result=wgpuDeviceCreateCommandEncoder((WGPUDevice)gpuHandle(object),&p0);return wrapGPU(context,@"commandEncoder",result,[](void *p){wgpuCommandEncoderRelease((WGPUCommandEncoder)p);});
 }
 if([op isEqualToString:@"device.createComputePipeline"]){
 auto p0=readComputePipelineDescriptor([args valueAtIndex:0],a);
 auto result=wgpuDeviceCreateComputePipeline((WGPUDevice)gpuHandle(object),&p0);return wrapGPU(context,@"computePipeline",result,[](void *p){wgpuComputePipelineRelease((WGPUComputePipeline)p);});
 }
 if([op isEqualToString:@"device.createPipelineLayout"]){
 auto p0=readPipelineLayoutDescriptor([args valueAtIndex:0],a);
 auto result=wgpuDeviceCreatePipelineLayout((WGPUDevice)gpuHandle(object),&p0);return wrapGPU(context,@"pipelineLayout",result,[](void *p){wgpuPipelineLayoutRelease((WGPUPipelineLayout)p);});
 }
 if([op isEqualToString:@"device.createQuerySet"]){
 auto p0=readQuerySetDescriptor([args valueAtIndex:0],a);
 auto result=wgpuDeviceCreateQuerySet((WGPUDevice)gpuHandle(object),&p0);return wrapGPU(context,@"querySet",result,[](void *p){wgpuQuerySetRelease((WGPUQuerySet)p);});
 }
 if([op isEqualToString:@"device.createRenderPipeline"]){
 auto p0=readRenderPipelineDescriptor([args valueAtIndex:0],a);
 auto result=wgpuDeviceCreateRenderPipeline((WGPUDevice)gpuHandle(object),&p0);return wrapGPU(context,@"renderPipeline",result,[](void *p){wgpuRenderPipelineRelease((WGPURenderPipeline)p);});
 }
 if([op isEqualToString:@"device.createSampler"]){
 auto p0=readSamplerDescriptor([args valueAtIndex:0],a);
 auto result=wgpuDeviceCreateSampler((WGPUDevice)gpuHandle(object),&p0);return wrapGPU(context,@"sampler",result,[](void *p){wgpuSamplerRelease((WGPUSampler)p);});
 }
 if([op isEqualToString:@"device.createShaderModule"]){
 auto p0=readShaderModuleDescriptor([args valueAtIndex:0],a);
 auto result=wgpuDeviceCreateShaderModule((WGPUDevice)gpuHandle(object),&p0);return wrapGPU(context,@"shaderModule",result,[](void *p){wgpuShaderModuleRelease((WGPUShaderModule)p);});
 }
 if([op isEqualToString:@"device.createTexture"]){
 auto p0=readTextureDescriptor([args valueAtIndex:0],a);
 auto result=wgpuDeviceCreateTexture((WGPUDevice)gpuHandle(object),&p0);return wrapGPU(context,@"texture",result,[](void *p){wgpuTextureRelease((WGPUTexture)p);});
 }
 if([op isEqualToString:@"texture.createView"]){
 auto p0=readTextureViewDescriptor([args valueAtIndex:0],a);
 auto result=wgpuTextureCreateView((WGPUTexture)gpuHandle(object),&p0);return wrapGPU(context,@"textureView",result,[](void *p){wgpuTextureViewRelease((WGPUTextureView)p);});
 }
 if([op isEqualToString:@"texture.destroy"]){
 wgpuTextureDestroy((WGPUTexture)gpuHandle(object));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"buffer.unmap"]){
 wgpuBufferUnmap((WGPUBuffer)gpuHandle(object));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"buffer.destroy"]){
 wgpuBufferDestroy((WGPUBuffer)gpuHandle(object));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPipeline.getBindGroupLayout"]){
 auto result=wgpuRenderPipelineGetBindGroupLayout((WGPURenderPipeline)gpuHandle(object),(uint32_t)[[args valueAtIndex:0] toDouble]);return wrapGPU(context,@"bindGroupLayout",result,[](void *p){wgpuBindGroupLayoutRelease((WGPUBindGroupLayout)p);});
 }
 if([op isEqualToString:@"computePipeline.getBindGroupLayout"]){
 auto result=wgpuComputePipelineGetBindGroupLayout((WGPUComputePipeline)gpuHandle(object),(uint32_t)[[args valueAtIndex:0] toDouble]);return wrapGPU(context,@"bindGroupLayout",result,[](void *p){wgpuBindGroupLayoutRelease((WGPUBindGroupLayout)p);});
 }
 if([op isEqualToString:@"commandEncoder.finish"]){
 auto p0=readCommandBufferDescriptor([args valueAtIndex:0],a);
 auto result=wgpuCommandEncoderFinish((WGPUCommandEncoder)gpuHandle(object),&p0);return wrapGPU(context,@"commandBuffer",result,[](void *p){wgpuCommandBufferRelease((WGPUCommandBuffer)p);});
 }
 if([op isEqualToString:@"commandEncoder.beginComputePass"]){
 auto p0=readComputePassDescriptor([args valueAtIndex:0],a);
 auto result=wgpuCommandEncoderBeginComputePass((WGPUCommandEncoder)gpuHandle(object),&p0);return wrapGPU(context,@"computePassEncoder",result,[](void *p){wgpuComputePassEncoderRelease((WGPUComputePassEncoder)p);});
 }
 if([op isEqualToString:@"commandEncoder.beginRenderPass"]){
 auto p0=readRenderPassDescriptor([args valueAtIndex:0],a);
 auto result=wgpuCommandEncoderBeginRenderPass((WGPUCommandEncoder)gpuHandle(object),&p0);return wrapGPU(context,@"renderPassEncoder",result,[](void *p){wgpuRenderPassEncoderRelease((WGPURenderPassEncoder)p);});
 }
 if([op isEqualToString:@"commandEncoder.copyBufferToBuffer"]){
 wgpuCommandEncoderCopyBufferToBuffer((WGPUCommandEncoder)gpuHandle(object),(WGPUBuffer)gpuHandle([args valueAtIndex:0]),(uint64_t)[[args valueAtIndex:1] toDouble],(WGPUBuffer)gpuHandle([args valueAtIndex:2]),(uint64_t)[[args valueAtIndex:3] toDouble],(uint64_t)[[args valueAtIndex:4] toDouble]);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"commandEncoder.copyBufferToTexture"]){
 auto p0=readTexelCopyBufferInfo([args valueAtIndex:0],a);
 auto p1=readTexelCopyTextureInfo([args valueAtIndex:1],a);
 auto p2=readExtent3D([args valueAtIndex:2],a);
 wgpuCommandEncoderCopyBufferToTexture((WGPUCommandEncoder)gpuHandle(object),&p0,&p1,&p2);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"commandEncoder.copyTextureToBuffer"]){
 auto p0=readTexelCopyTextureInfo([args valueAtIndex:0],a);
 auto p1=readTexelCopyBufferInfo([args valueAtIndex:1],a);
 auto p2=readExtent3D([args valueAtIndex:2],a);
 wgpuCommandEncoderCopyTextureToBuffer((WGPUCommandEncoder)gpuHandle(object),&p0,&p1,&p2);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"commandEncoder.copyTextureToTexture"]){
 auto p0=readTexelCopyTextureInfo([args valueAtIndex:0],a);
 auto p1=readTexelCopyTextureInfo([args valueAtIndex:1],a);
 auto p2=readExtent3D([args valueAtIndex:2],a);
 wgpuCommandEncoderCopyTextureToTexture((WGPUCommandEncoder)gpuHandle(object),&p0,&p1,&p2);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"commandEncoder.clearBuffer"]){
 wgpuCommandEncoderClearBuffer((WGPUCommandEncoder)gpuHandle(object),(WGPUBuffer)gpuHandle([args valueAtIndex:0]),(present([args valueAtIndex:1])?(uint64_t)[[args valueAtIndex:1] toDouble]:(uint64_t)0),(present([args valueAtIndex:2])?(uint64_t)[[args valueAtIndex:2] toDouble]:(uint64_t)WGPU_WHOLE_SIZE));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"commandEncoder.resolveQuerySet"]){
 wgpuCommandEncoderResolveQuerySet((WGPUCommandEncoder)gpuHandle(object),(WGPUQuerySet)gpuHandle([args valueAtIndex:0]),(uint32_t)[[args valueAtIndex:1] toDouble],(uint32_t)[[args valueAtIndex:2] toDouble],(WGPUBuffer)gpuHandle([args valueAtIndex:3]),(uint64_t)[[args valueAtIndex:4] toDouble]);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.setPipeline"]){
 wgpuRenderPassEncoderSetPipeline((WGPURenderPassEncoder)gpuHandle(object),(WGPURenderPipeline)gpuHandle([args valueAtIndex:0]));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.setBindGroup"]){
 JSValue *list3=[args valueAtIndex:2];size_t count3=present(list3)?list3[@"length"].toUInt32:0;auto *p3=a.alloc<uint32_t>(count3);for(size_t i=0;i<count3;i++)p3[i]=(uint32_t)[[list3 valueAtIndex:i] toDouble];
 wgpuRenderPassEncoderSetBindGroup((WGPURenderPassEncoder)gpuHandle(object),(uint32_t)[[args valueAtIndex:0] toDouble],(WGPUBindGroup)gpuHandle([args valueAtIndex:1]),count3,p3);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.draw"]){
 wgpuRenderPassEncoderDraw((WGPURenderPassEncoder)gpuHandle(object),(uint32_t)[[args valueAtIndex:0] toDouble],(present([args valueAtIndex:1])?(uint32_t)[[args valueAtIndex:1] toDouble]:(uint32_t)1),(present([args valueAtIndex:2])?(uint32_t)[[args valueAtIndex:2] toDouble]:(uint32_t)0),(present([args valueAtIndex:3])?(uint32_t)[[args valueAtIndex:3] toDouble]:(uint32_t)0));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.drawIndexed"]){
 wgpuRenderPassEncoderDrawIndexed((WGPURenderPassEncoder)gpuHandle(object),(uint32_t)[[args valueAtIndex:0] toDouble],(present([args valueAtIndex:1])?(uint32_t)[[args valueAtIndex:1] toDouble]:(uint32_t)1),(present([args valueAtIndex:2])?(uint32_t)[[args valueAtIndex:2] toDouble]:(uint32_t)0),(present([args valueAtIndex:3])?(int32_t)[[args valueAtIndex:3] toDouble]:(int32_t)0),(present([args valueAtIndex:4])?(uint32_t)[[args valueAtIndex:4] toDouble]:(uint32_t)0));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.drawIndirect"]){
 wgpuRenderPassEncoderDrawIndirect((WGPURenderPassEncoder)gpuHandle(object),(WGPUBuffer)gpuHandle([args valueAtIndex:0]),(uint64_t)[[args valueAtIndex:1] toDouble]);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.drawIndexedIndirect"]){
 wgpuRenderPassEncoderDrawIndexedIndirect((WGPURenderPassEncoder)gpuHandle(object),(WGPUBuffer)gpuHandle([args valueAtIndex:0]),(uint64_t)[[args valueAtIndex:1] toDouble]);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.setStencilReference"]){
 wgpuRenderPassEncoderSetStencilReference((WGPURenderPassEncoder)gpuHandle(object),(uint32_t)[[args valueAtIndex:0] toDouble]);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.setBlendConstant"]){
 auto p0=readColor([args valueAtIndex:0],a);
 wgpuRenderPassEncoderSetBlendConstant((WGPURenderPassEncoder)gpuHandle(object),&p0);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.setViewport"]){
 wgpuRenderPassEncoderSetViewport((WGPURenderPassEncoder)gpuHandle(object),(float)[[args valueAtIndex:0] toDouble],(float)[[args valueAtIndex:1] toDouble],(float)[[args valueAtIndex:2] toDouble],(float)[[args valueAtIndex:3] toDouble],(float)[[args valueAtIndex:4] toDouble],(float)[[args valueAtIndex:5] toDouble]);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.setScissorRect"]){
 wgpuRenderPassEncoderSetScissorRect((WGPURenderPassEncoder)gpuHandle(object),(uint32_t)[[args valueAtIndex:0] toDouble],(uint32_t)[[args valueAtIndex:1] toDouble],(uint32_t)[[args valueAtIndex:2] toDouble],(uint32_t)[[args valueAtIndex:3] toDouble]);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.setVertexBuffer"]){
 wgpuRenderPassEncoderSetVertexBuffer((WGPURenderPassEncoder)gpuHandle(object),(uint32_t)[[args valueAtIndex:0] toDouble],(WGPUBuffer)gpuHandle([args valueAtIndex:1]),(present([args valueAtIndex:2])?(uint64_t)[[args valueAtIndex:2] toDouble]:(uint64_t)0),(present([args valueAtIndex:3])?(uint64_t)[[args valueAtIndex:3] toDouble]:(uint64_t)WGPU_WHOLE_SIZE));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.setIndexBuffer"]){
 wgpuRenderPassEncoderSetIndexBuffer((WGPURenderPassEncoder)gpuHandle(object),(WGPUBuffer)gpuHandle([args valueAtIndex:0]),enumIndexFormat([args valueAtIndex:1]),(present([args valueAtIndex:2])?(uint64_t)[[args valueAtIndex:2] toDouble]:(uint64_t)0),(present([args valueAtIndex:3])?(uint64_t)[[args valueAtIndex:3] toDouble]:(uint64_t)WGPU_WHOLE_SIZE));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.beginOcclusionQuery"]){
 wgpuRenderPassEncoderBeginOcclusionQuery((WGPURenderPassEncoder)gpuHandle(object),(uint32_t)[[args valueAtIndex:0] toDouble]);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.endOcclusionQuery"]){
 wgpuRenderPassEncoderEndOcclusionQuery((WGPURenderPassEncoder)gpuHandle(object));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"renderPassEncoder.end"]){
 wgpuRenderPassEncoderEnd((WGPURenderPassEncoder)gpuHandle(object));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"computePassEncoder.setPipeline"]){
 wgpuComputePassEncoderSetPipeline((WGPUComputePassEncoder)gpuHandle(object),(WGPUComputePipeline)gpuHandle([args valueAtIndex:0]));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"computePassEncoder.setBindGroup"]){
 JSValue *list3=[args valueAtIndex:2];size_t count3=present(list3)?list3[@"length"].toUInt32:0;auto *p3=a.alloc<uint32_t>(count3);for(size_t i=0;i<count3;i++)p3[i]=(uint32_t)[[list3 valueAtIndex:i] toDouble];
 wgpuComputePassEncoderSetBindGroup((WGPUComputePassEncoder)gpuHandle(object),(uint32_t)[[args valueAtIndex:0] toDouble],(WGPUBindGroup)gpuHandle([args valueAtIndex:1]),count3,p3);return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"computePassEncoder.dispatchWorkgroups"]){
 wgpuComputePassEncoderDispatchWorkgroups((WGPUComputePassEncoder)gpuHandle(object),(uint32_t)[[args valueAtIndex:0] toDouble],(present([args valueAtIndex:1])?(uint32_t)[[args valueAtIndex:1] toDouble]:(uint32_t)1),(present([args valueAtIndex:2])?(uint32_t)[[args valueAtIndex:2] toDouble]:(uint32_t)1));return [JSValue valueWithUndefinedInContext:context];
 }
 if([op isEqualToString:@"computePassEncoder.end"]){
 wgpuComputePassEncoderEnd((WGPUComputePassEncoder)gpuHandle(object));return [JSValue valueWithUndefinedInContext:context];
 }
 return nil;
}
