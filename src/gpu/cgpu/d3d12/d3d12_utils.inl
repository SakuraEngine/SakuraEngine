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

FORCEINLINE static D3D12_RASTERIZER_DESC D3D12Util_TranslateRasterizerState(const CGpuRasterizerStateDescriptor *pDesc) {
  cgpu_assert(pDesc->fill_mode < FM_COUNT);
  cgpu_assert(pDesc->cull_mode < CULL_MODE_COUNT);
  cgpu_assert(pDesc->front_face == FF_CCW ||  pDesc->front_face == FF_CW);
  D3D12_RASTERIZER_DESC ret = {};
  ret.FillMode = gDx12FillModeTranslator[pDesc->fill_mode];
  ret.CullMode = gDx12CullModeTranslator[pDesc->cull_mode];
  ret.FrontCounterClockwise = pDesc->front_face == FF_CCW;
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
  cgpu_assert(pDesc->depth_func < MAX_COMPARE_MODES);
  cgpu_assert(pDesc->stencil_front_func < MAX_COMPARE_MODES);
  cgpu_assert(pDesc->stencil_front_fail < MAX_STENCIL_OPS);
  cgpu_assert(pDesc->depth_front_fail < MAX_STENCIL_OPS);
  cgpu_assert(pDesc->stencil_front_pass < MAX_STENCIL_OPS);
  cgpu_assert(pDesc->stencil_back_func < MAX_COMPARE_MODES);
  cgpu_assert(pDesc->stencil_back_fail < MAX_STENCIL_OPS);
  cgpu_assert(pDesc->depth_back_fail < MAX_STENCIL_OPS);
  cgpu_assert(pDesc->stencil_back_pass < MAX_STENCIL_OPS);

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
  case TOPO_POINT_LIST:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
  case TOPO_LINE_LIST:
  case TOPO_LINE_STRIP:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
  case TOPO_TRI_LIST:
  case TOPO_TRI_STRIP:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  case TOPO_PATCH_LIST:
    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
  }
  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}

FORCEINLINE static D3D12_RESOURCE_STATES D3D12Util_TranslateResourceState(ECGpuResourceState state)
{
    D3D12_RESOURCE_STATES ret = D3D12_RESOURCE_STATE_COMMON;

    // These states cannot be combined with other states so we just do an == check
    if (state == RS_GENERIC_READ)
        return D3D12_RESOURCE_STATE_GENERIC_READ;
    if (state == RS_COMMON)
        return D3D12_RESOURCE_STATE_COMMON;
    if (state == RS_PRESENT)
        return D3D12_RESOURCE_STATE_PRESENT;

    if (state & RS_VERTEX_AND_CONSTANT_BUFFER)
        ret |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if (state & RS_INDEX_BUFFER)
        ret |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
    if (state & RS_RENDER_TARGET)
        ret |= D3D12_RESOURCE_STATE_RENDER_TARGET;
    if (state & RS_UNORDERED_ACCESS)
        ret |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    if (state & RS_DEPTH_WRITE)
        ret |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
    if (state & RS_DEPTH_READ)
        ret |= D3D12_RESOURCE_STATE_DEPTH_READ;
    if (state & RS_STREAM_OUT)
        ret |= D3D12_RESOURCE_STATE_STREAM_OUT;
    if (state & RS_INDIRECT_ARGUMENT)
        ret |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
    if (state & RS_COPY_DEST)
        ret |= D3D12_RESOURCE_STATE_COPY_DEST;
    if (state & RS_COPY_SOURCE)
        ret |= D3D12_RESOURCE_STATE_COPY_SOURCE;
    if (state & RS_NON_PIXEL_SHADER_RESOURCE)
        ret |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    if (state & RS_PIXEL_SHADER_RESOURCE)
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
  if (stages == SS_COMPUTE) {
    return D3D12_SHADER_VISIBILITY_ALL;
  }
  if (stages & SS_VERT) {
    res = D3D12_SHADER_VISIBILITY_VERTEX;
    ++stageCount;
  }
  if (stages & SS_GEOM) {
    res = D3D12_SHADER_VISIBILITY_GEOMETRY;
    ++stageCount;
  }
  if (stages & SS_HULL) {
    res = D3D12_SHADER_VISIBILITY_HULL;
    ++stageCount;
  }
  if (stages & SS_DOMN) {
    res = D3D12_SHADER_VISIBILITY_DOMAIN;
    ++stageCount;
  }
  if (stages & SS_FRAG) {
    res = D3D12_SHADER_VISIBILITY_PIXEL;
    ++stageCount;
  }
  if (stages == SS_RAYTRACING) {
    return D3D12_SHADER_VISIBILITY_ALL;
  }
  cgpu_assert(stageCount > 0);
  return stageCount > 1 ? D3D12_SHADER_VISIBILITY_ALL : res;
}


FORCEINLINE static DXGI_FORMAT DXGIUtil_TranslatePixelFormat(const ECGpuFormat fmt)
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
	case PF_D16_UNORM: return DXGI_FORMAT_D16_UNORM;
	case PF_X8_D24_UNORM: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case PF_D32_SFLOAT: return DXGI_FORMAT_D32_FLOAT;
	case PF_D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case PF_D32_SFLOAT_S8_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
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