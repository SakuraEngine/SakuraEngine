/* clang-format off */
FORCEINLINE static void D3D12Util_CopyDescriptorHandle(D3D12Util_DescriptorHeap *pHeap,
                                   const D3D12_CPU_DESCRIPTOR_HANDLE &srcHandle,
                                   const uint64_t &dstHandle, uint32_t index) {
  pHeap->pHandles[(dstHandle / pHeap->mDescriptorSize) + index] = srcHandle;
  pHeap->pDevice->CopyDescriptorsSimple(1,
                                        {pHeap->mStartHandle.mCpu.ptr +
                                         dstHandle +
                                         (index * pHeap->mDescriptorSize)},
                                        srcHandle, pHeap->mDesc.Type);
}

FORCEINLINE static void D3D12Util_CreateSRV(CGpuDevice_D3D12* D, ID3D12Resource* pResource,
    const D3D12_SHADER_RESOURCE_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
{
    if (D3D12_GPU_VIRTUAL_ADDRESS_NULL == pHandle->ptr)
        *pHandle = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1).mCpu;
    D->pDxDevice->CreateShaderResourceView(pResource, pSrvDesc, *pHandle);
}

FORCEINLINE static void D3D12Util_CreateUAV(CGpuDevice_D3D12* D, ID3D12Resource* pResource,
    ID3D12Resource* pCounterResource,
    const D3D12_UNORDERED_ACCESS_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
{
    if (D3D12_GPU_VIRTUAL_ADDRESS_NULL == pHandle->ptr)
        *pHandle = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1).mCpu;
    D->pDxDevice->CreateUnorderedAccessView(pResource, pCounterResource, pSrvDesc, *pHandle);
}

FORCEINLINE static void D3D12Util_CreateCBV(CGpuDevice_D3D12* D,
    const D3D12_CONSTANT_BUFFER_VIEW_DESC* pSrvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
{
    if (D3D12_GPU_VIRTUAL_ADDRESS_NULL == pHandle->ptr)
        *pHandle = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], 1).mCpu;
    D->pDxDevice->CreateConstantBufferView(pSrvDesc, *pHandle);
}

FORCEINLINE static void D3D12Util_CreateRTV(CGpuDevice_D3D12* D,
    ID3D12Resource* pResource,
    const D3D12_RENDER_TARGET_VIEW_DESC* pRtvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
{
    if (D3D12_GPU_VIRTUAL_ADDRESS_NULL == pHandle->ptr)
        *pHandle = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV], 1).mCpu;
    D->pDxDevice->CreateRenderTargetView(pResource, pRtvDesc, *pHandle);
}

FORCEINLINE static void D3D12Util_CreateDSV(CGpuDevice_D3D12* D,
    ID3D12Resource* pResource,
    const D3D12_DEPTH_STENCIL_VIEW_DESC* pDsvDesc, D3D12_CPU_DESCRIPTOR_HANDLE* pHandle)
{
    if (D3D12_GPU_VIRTUAL_ADDRESS_NULL == pHandle->ptr)
        *pHandle = D3D12Util_ConsumeDescriptorHandles(D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_DSV], 1).mCpu;
    D->pDxDevice->CreateDepthStencilView(pResource, pDsvDesc, *pHandle);
}

FORCEINLINE static D3D12_DESCRIPTOR_RANGE_TYPE D3D12Util_ResourceTypeToDescriptorRangeType(ECGpuResourceType type) {
  switch (type) {
  case RT_UNIFORM_BUFFER:
  case RT_ROOT_CONSTANT:
    return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
  case RT_RW_BUFFER:
  case RT_RW_TEXTURE:
    return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
  case RT_SAMPLER:
    return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
  case RT_RAY_TRACING:
  case RT_TEXTURE:
  case RT_BUFFER:
    return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  default:
    cgpu_assert(false && "Invalid DescriptorInfo Type");
    return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
  }
}

FORCEINLINE static D3D12_BLEND_DESC D3D12Util_TranslateBlendState(const CGpuBlendStateDescriptor* pDesc)
{
  int blendDescIndex = 0;
  D3D12_BLEND_DESC ret = {};
  ret.AlphaToCoverageEnable = (BOOL)pDesc->alpha_to_coverage;
  ret.IndependentBlendEnable = TRUE;
  for (int i = 0; i < MAX_MRT_COUNT; i++) {
      BOOL blendEnable =
        (gDx12BlendConstantTranslator[pDesc->src_factors[blendDescIndex]] !=
            D3D12_BLEND_ONE ||
        gDx12BlendConstantTranslator[pDesc->dst_factors[blendDescIndex]] !=
            D3D12_BLEND_ZERO ||
        gDx12BlendConstantTranslator[pDesc->src_alpha_factors[blendDescIndex]] !=
            D3D12_BLEND_ONE ||
           gDx12BlendConstantTranslator[pDesc->dst_alpha_factors[blendDescIndex]] !=
            D3D12_BLEND_ZERO);

      ret.RenderTarget[i].BlendEnable = blendEnable;
      ret.RenderTarget[i].RenderTargetWriteMask = (UINT8)pDesc->masks[blendDescIndex];
      ret.RenderTarget[i].BlendOp =
          gDx12BlendOpTranslator[pDesc->blend_modes[blendDescIndex]];
      ret.RenderTarget[i].SrcBlend =
          gDx12BlendConstantTranslator[pDesc->src_factors[blendDescIndex]];
      ret.RenderTarget[i].DestBlend =
          gDx12BlendConstantTranslator[pDesc->dst_factors[blendDescIndex]];
      ret.RenderTarget[i].BlendOpAlpha =
          gDx12BlendOpTranslator[pDesc->blend_alpha_modes[blendDescIndex]];
      ret.RenderTarget[i].SrcBlendAlpha =
          gDx12BlendConstantTranslator[pDesc->src_alpha_factors[blendDescIndex]];
      ret.RenderTarget[i].DestBlendAlpha =
          gDx12BlendConstantTranslator[pDesc->dst_alpha_factors[blendDescIndex]];

    if (pDesc->independent_blend)
      ++blendDescIndex;
  }
  return ret;
}

FORCEINLINE static D3D12_FILTER D3D12Util_TranslateFilter(ECGpuFilterType minFilter, ECGpuFilterType magFilter,
    ECGpuMipMapMode mipMapMode, bool aniso, bool comparisonFilterEnabled) 
{
  if (aniso)
    return (comparisonFilterEnabled ? D3D12_FILTER_COMPARISON_ANISOTROPIC
                                    : D3D12_FILTER_ANISOTROPIC);

  // control bit : minFilter  magFilter   mipMapMode
  //   point   :   00	  00	   00
  //   linear  :   01	  01	   01
  // ex : trilinear == 010101
  int filter = (minFilter << 4) | (magFilter << 2) | mipMapMode;
  int baseFilter = comparisonFilterEnabled
                       ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT
                       : D3D12_FILTER_MIN_MAG_MIP_POINT;
  return (D3D12_FILTER)(baseFilter + filter);
}

D3D12_TEXTURE_ADDRESS_MODE D3D12Util_TranslateAddressMode(ECGpuAddressMode addressMode) 
{
  switch (addressMode) {
  case ADDRESS_MODE_MIRROR:
    return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
  case ADDRESS_MODE_REPEAT:
    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  case ADDRESS_MODE_CLAMP_TO_EDGE:
    return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
  case ADDRESS_MODE_CLAMP_TO_BORDER:
    return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
  default:
    return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  }
}

FORCEINLINE static D3D12_RASTERIZER_DESC D3D12Util_TranslateRasterizerState(const CGpuRasterizerStateDescriptor *pDesc) {
  cgpu_assert(pDesc->fill_mode < FILL_MODE_COUNT);
  cgpu_assert(pDesc->cull_mode < CULL_MODE_COUNT);
  cgpu_assert(pDesc->front_face == FRONT_FACE_CCW ||  pDesc->front_face == FRONT_FACE_CW);
  D3D12_RASTERIZER_DESC ret = {};
  ret.FillMode = gDx12FillModeTranslator[pDesc->fill_mode];
  ret.CullMode = gDx12CullModeTranslator[pDesc->cull_mode];
  ret.FrontCounterClockwise = pDesc->front_face == FRONT_FACE_CCW;
  ret.DepthBias = pDesc->depth_bias;
  ret.DepthBiasClamp = 0.0f;
  ret.SlopeScaledDepthBias = pDesc->slope_scaled_depth_bias;
  ret.DepthClipEnable = !pDesc->enable_depth_clamp;
  ret.MultisampleEnable = pDesc->enable_multi_sample ? TRUE : FALSE;
  ret.AntialiasedLineEnable = FALSE;
  ret.ForcedSampleCount = 0;
  ret.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
  return ret;
}

FORCEINLINE static  D3D12_DEPTH_STENCIL_DESC D3D12Util_TranslateDephStencilState(const CGpuDepthStateDescriptor *pDesc) {
  cgpu_assert(pDesc->depth_func < CMP_COUNT);
  cgpu_assert(pDesc->stencil_front_func < CMP_COUNT);
  cgpu_assert(pDesc->stencil_front_fail < STENCIL_OP_COUNT);
  cgpu_assert(pDesc->depth_front_fail < STENCIL_OP_COUNT);
  cgpu_assert(pDesc->stencil_front_pass < STENCIL_OP_COUNT);
  cgpu_assert(pDesc->stencil_back_func < CMP_COUNT);
  cgpu_assert(pDesc->stencil_back_fail < STENCIL_OP_COUNT);
  cgpu_assert(pDesc->depth_back_fail < STENCIL_OP_COUNT);
  cgpu_assert(pDesc->stencil_back_pass < STENCIL_OP_COUNT);

  D3D12_DEPTH_STENCIL_DESC ret = {};
  ret.DepthEnable = (BOOL)pDesc->depth_test;
  ret.DepthWriteMask = pDesc->depth_write ? D3D12_DEPTH_WRITE_MASK_ALL
                                          : D3D12_DEPTH_WRITE_MASK_ZERO;
  ret.DepthFunc = gDx12ComparisonFuncTranslator[pDesc->depth_func];
  ret.StencilEnable = (BOOL)pDesc->stencil_test;
  ret.StencilReadMask = pDesc->stencil_read_mask;
  ret.StencilWriteMask = pDesc->stencil_write_mask;
  ret.BackFace.StencilFunc =
      gDx12ComparisonFuncTranslator[pDesc->stencil_back_func];
  ret.FrontFace.StencilFunc =
      gDx12ComparisonFuncTranslator[pDesc->stencil_front_func];
  ret.BackFace.StencilDepthFailOp =
      gDx12StencilOpTranslator[pDesc->depth_back_fail];
  ret.FrontFace.StencilDepthFailOp =
      gDx12StencilOpTranslator[pDesc->depth_front_fail];
  ret.BackFace.StencilFailOp =
      gDx12StencilOpTranslator[pDesc->stencil_back_fail];
  ret.FrontFace.StencilFailOp =
      gDx12StencilOpTranslator[pDesc->stencil_front_fail];
  ret.BackFace.StencilPassOp =
      gDx12StencilOpTranslator[pDesc->stencil_back_pass];
  ret.FrontFace.StencilPassOp =
      gDx12StencilOpTranslator[pDesc->stencil_front_pass];

  return ret;
}

FORCEINLINE static D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12Util_TranslatePrimitiveTopology(ECGpuPrimitiveTopology topology) {
  switch (topology) {
  case PRIM_TOPO_POINT_LIST:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
  case PRIM_TOPO_LINE_LIST:
  case PRIM_TOPO_LINE_STRIP:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
  case PRIM_TOPO_TRI_LIST:
  case PRIM_TOPO_TRI_STRIP:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  case PRIM_TOPO_PATCH_LIST:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
  }
  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}

FORCEINLINE static D3D12_RESOURCE_STATES D3D12Util_TranslateResourceState(CGpuResourceStates state)
{
    D3D12_RESOURCE_STATES ret = D3D12_RESOURCE_STATE_COMMON;

    // These states cannot be combined with other states so we just do an == check
    if (state == RESOURCE_STATE_GENERIC_READ)
        return D3D12_RESOURCE_STATE_GENERIC_READ;
    if (state == RESOURCE_STATE_COMMON)
        return D3D12_RESOURCE_STATE_COMMON;
    if (state == RESOURCE_STATE_PRESENT)
        return D3D12_RESOURCE_STATE_PRESENT;

    if (state & RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
        ret |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if (state & RESOURCE_STATE_RESOLVE_DEST)
        ret |= D3D12_RESOURCE_STATE_RESOLVE_DEST;
    if (state & RESOURCE_STATE_INDEX_BUFFER)
        ret |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    if (state & RESOURCE_STATE_RENDER_TARGET)
        ret |= D3D12_RESOURCE_STATE_RENDER_TARGET;
    if (state & RESOURCE_STATE_UNORDERED_ACCESS)
        ret |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    if (state & RESOURCE_STATE_DEPTH_WRITE)
        ret |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
    if (state & RESOURCE_STATE_DEPTH_READ)
        ret |= D3D12_RESOURCE_STATE_DEPTH_READ;
    if (state & RESOURCE_STATE_STREAM_OUT)
        ret |= D3D12_RESOURCE_STATE_STREAM_OUT;
    if (state & RESOURCE_STATE_INDIRECT_ARGUMENT)
        ret |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    if (state & RESOURCE_STATE_COPY_DEST)
        ret |= D3D12_RESOURCE_STATE_COPY_DEST;
    if (state & RESOURCE_STATE_COPY_SOURCE)
        ret |= D3D12_RESOURCE_STATE_COPY_SOURCE;
    if (state & RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
        ret |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    if (state & RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
        ret |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
#ifdef ENABLE_RAYTRACING
    if (state & RS_RAYTRACING_ACCELERATION_STRUCTURE)
        ret |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
#endif
#ifdef ENABLE_VRS
    if (state & RS_SHADING_RATE_SOURCE)
        ret |= D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
#endif
    return ret;
}

FORCEINLINE static D3D12_SHADER_VISIBILITY D3D12Util_TranslateShaderStages(CGpuShaderStages stages) {
  D3D12_SHADER_VISIBILITY res = D3D12_SHADER_VISIBILITY_ALL;
  uint32_t stageCount = 0;
  if (stages == SHADER_STAGE_COMPUTE) {
    return D3D12_SHADER_VISIBILITY_ALL;
  }
  if (stages & SHADER_STAGE_VERT) {
    res = D3D12_SHADER_VISIBILITY_VERTEX;
    ++stageCount;
  }
  if (stages & SHADER_STAGE_GEOM) {
    res = D3D12_SHADER_VISIBILITY_GEOMETRY;
    ++stageCount;
  }
  if (stages & SHADER_STAGE_ALL_HULL) {
    res = D3D12_SHADER_VISIBILITY_HULL;
    ++stageCount;
  }
  if (stages & SHADER_STAGE_ALL_DOMAIN) {
    res = D3D12_SHADER_VISIBILITY_DOMAIN;
    ++stageCount;
  }
  if (stages & SHADER_STAGE_FRAG) {
    res = D3D12_SHADER_VISIBILITY_PIXEL;
    ++stageCount;
  }
  if (stages == SHADER_STAGE_RAYTRACING) {
    return D3D12_SHADER_VISIBILITY_ALL;
  }
  cgpu_assert(stageCount > 0);
  return stageCount > 1 ? D3D12_SHADER_VISIBILITY_ALL : res;
}


FORCEINLINE static DXGI_FORMAT DXGIUtil_TranslatePixelFormat(const ECGpuFormat fmt, bool ShaderResource = false)
{
	switch (fmt) {
	case PF_R1_UNORM: return DXGI_FORMAT_R1_UNORM;
	case PF_R5G6B5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
	case PF_B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
	case PF_B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
	case PF_R8_UNORM: return DXGI_FORMAT_R8_UNORM;
	case PF_R8_SNORM: return DXGI_FORMAT_R8_SNORM;
	case PF_R8_UINT: return DXGI_FORMAT_R8_UINT;
	case PF_R8_SINT: return DXGI_FORMAT_R8_SINT;
	case PF_R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
	case PF_R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
	case PF_R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
	case PF_R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
	case PF_B4G4R4A4_UNORM: return DXGI_FORMAT_B4G4R4A4_UNORM;

	case PF_R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case PF_R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
	case PF_R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
	case PF_R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
	case PF_R8G8B8A8_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	case PF_B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case PF_B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
	case PF_B8G8R8A8_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

	case PF_R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
	case PF_R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;

	case PF_R16_UNORM: return DXGI_FORMAT_R16_UNORM;
	case PF_R16_SNORM: return DXGI_FORMAT_R16_SNORM;
	case PF_R16_UINT: return DXGI_FORMAT_R16_UINT;
	case PF_R16_SINT: return DXGI_FORMAT_R16_SINT;
	case PF_R16_SFLOAT: return DXGI_FORMAT_R16_FLOAT;
	case PF_R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
	case PF_R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
	case PF_R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
	case PF_R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
	case PF_R16G16_SFLOAT: return DXGI_FORMAT_R16G16_FLOAT;
	case PF_R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
	case PF_R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
	case PF_R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
	case PF_R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
	case PF_R16G16B16A16_SFLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case PF_R32_UINT: return DXGI_FORMAT_R32_UINT;
	case PF_R32_SINT: return DXGI_FORMAT_R32_SINT;
	case PF_R32_SFLOAT: return DXGI_FORMAT_R32_FLOAT;
	case PF_R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
	case PF_R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
	case PF_R32G32_SFLOAT: return DXGI_FORMAT_R32G32_FLOAT;
	case PF_R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
	case PF_R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
	case PF_R32G32B32_SFLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
	case PF_R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
	case PF_R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
	case PF_R32G32B32A32_SFLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case PF_B10G11R11_UFLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
	case PF_E5B9G9R9_UFLOAT: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
	case PF_D16_UNORM: return ShaderResource ? DXGI_FORMAT_R16_UNORM : DXGI_FORMAT_D16_UNORM;
	case PF_X8_D24_UNORM: return ShaderResource ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_D24_UNORM_S8_UINT;
	case PF_D32_SFLOAT: return  ShaderResource ? DXGI_FORMAT_R32_FLOAT :DXGI_FORMAT_D32_FLOAT;
	case PF_D24_UNORM_S8_UINT: return  ShaderResource ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS :DXGI_FORMAT_D24_UNORM_S8_UINT;
	case PF_D32_SFLOAT_S8_UINT: return  ShaderResource ? DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS :DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	case PF_DXBC1_RGB_UNORM: return DXGI_FORMAT_BC1_UNORM;
	case PF_DXBC1_RGB_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
	case PF_DXBC1_RGBA_UNORM: return DXGI_FORMAT_BC1_UNORM;
	case PF_DXBC1_RGBA_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
	case PF_DXBC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
	case PF_DXBC2_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
	case PF_DXBC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
	case PF_DXBC3_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
	case PF_DXBC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
	case PF_DXBC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
	case PF_DXBC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
	case PF_DXBC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
	case PF_DXBC6H_UFLOAT: return DXGI_FORMAT_BC6H_UF16;
	case PF_DXBC6H_SFLOAT: return DXGI_FORMAT_BC6H_SF16;
	case PF_DXBC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
	case PF_DXBC7_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;

	case PF_D16_UNORM_S8_UINT:
	case PF_R4G4_UNORM: 
	default: return DXGI_FORMAT_UNKNOWN;
	}
	return DXGI_FORMAT_UNKNOWN;
}


FORCEINLINE static DXGI_FORMAT DXGIUtil_FormatToTypeless(DXGI_FORMAT fmt) {
	switch (fmt) {
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_TYPELESS;

	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_TYPELESS;

	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT: return DXGI_FORMAT_R32G32_TYPELESS;

	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_TYPELESS;

	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT: return DXGI_FORMAT_R16G16_TYPELESS;

	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT: return DXGI_FORMAT_R32_TYPELESS;
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT: return DXGI_FORMAT_R8G8_TYPELESS;
	case DXGI_FORMAT_B4G4R4A4_UNORM: // just treats a 16 raw bits
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT: return DXGI_FORMAT_R16_TYPELESS;
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT: return DXGI_FORMAT_R8_TYPELESS;
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB: return DXGI_FORMAT_BC1_TYPELESS;
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB: return DXGI_FORMAT_BC2_TYPELESS;
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB: return DXGI_FORMAT_BC3_TYPELESS;
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM: return DXGI_FORMAT_BC4_TYPELESS;
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM: return DXGI_FORMAT_BC5_TYPELESS;
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM: return DXGI_FORMAT_R16_TYPELESS;

	case DXGI_FORMAT_R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;

	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8X8_TYPELESS;

	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_TYPELESS;

	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16: return DXGI_FORMAT_BC6H_TYPELESS;

	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB: return DXGI_FORMAT_BC7_TYPELESS;

	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_R32G8X24_TYPELESS;
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return DXGI_FORMAT_R24G8_TYPELESS;
	case DXGI_FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_R24G8_TYPELESS;

		// typeless just return the input format
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC7_TYPELESS: return fmt;

	case DXGI_FORMAT_R1_UNORM:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_AYUV:
	case DXGI_FORMAT_Y410:
	case DXGI_FORMAT_Y416:
	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
	case DXGI_FORMAT_420_OPAQUE:
	case DXGI_FORMAT_YUY2:
	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
	case DXGI_FORMAT_NV11:
	case DXGI_FORMAT_AI44:
	case DXGI_FORMAT_IA44:
	case DXGI_FORMAT_P8:
	case DXGI_FORMAT_A8P8:
	case DXGI_FORMAT_P208:
	case DXGI_FORMAT_V208:
	case DXGI_FORMAT_V408:
	case DXGI_FORMAT_UNKNOWN: return DXGI_FORMAT_UNKNOWN;
	}
	return DXGI_FORMAT_UNKNOWN;
}