#pragma once
#include "cgpu/api.h"
#include "assert.h"
#include "dxgiformat.h"

#ifdef __cplusplus
extern "C" {
#endif
DXGI_FORMAT pf_translate_to_d3d12(const ECGpuPixelFormat form);



#define trans_case(_format) case PF_##_format: return DXGI_FORMAT_##_format;

inline DXGI_FORMAT pf_translate_to_d3d12(const ECGpuPixelFormat form)
{
    switch (form)
	{
	trans_case(R32G32B32A32_UINT)
	trans_case(R32G32B32A32_SINT)
	trans_case(R32G32B32A32_FLOAT)
	trans_case(R32G32B32_UINT)
	trans_case(R32G32B32_SINT)
	trans_case(R32G32B32_FLOAT)
	trans_case(R16G16B16A16_UNORM)
	trans_case(R16G16B16A16_SNORM)
	trans_case(R16G16B16A16_UINT)
	trans_case(R16G16B16A16_SINT)
	trans_case(R16G16B16A16_FLOAT)
	trans_case(R32G32_UINT)
	trans_case(R32G32_SINT)
	trans_case(R32G32_FLOAT)
	trans_case(R10G10B10A2_UNORM)
	trans_case(R10G10B10A2_UINT)
	//trans_case(R9G9B9E5_UFLOAT)
	trans_case(R8G8B8A8_UNORM)
	trans_case(R8G8B8A8_SNORM)
	trans_case(R8G8B8A8_UINT)
	trans_case(R8G8B8A8_SINT)
	trans_case(R8G8B8A8_UNORM_SRGB)
	trans_case(B8G8R8A8_UNORM)
	trans_case(B8G8R8A8_UNORM_SRGB)
	trans_case(R11G11B10_FLOAT)
	trans_case(R16G16_UNORM)
	trans_case(R16G16_SNORM)
	trans_case(R16G16_UINT)
	trans_case(R16G16_SINT)
	trans_case(R16G16_FLOAT)
	trans_case(R32_UINT)
	trans_case(R32_SINT)
	trans_case(R32_FLOAT)
	trans_case(B5G5R5A1_UNORM)
	trans_case(B5G6R5_UNORM)
	trans_case(R8G8_UNORM)
	trans_case(R8G8_SNORM)
	trans_case(R8G8_UINT)
	trans_case(R8G8_SINT)
	trans_case(R16_UNORM)
	trans_case(R16_SNORM)
	trans_case(R16_UINT)
	trans_case(R16_SINT)
	trans_case(R16_FLOAT)
	trans_case(R8_UNORM)
	trans_case(R8_SNORM)
	trans_case(R8_UINT)
	trans_case(R8_SINT)
	trans_case(D24_UNORM_S8_UINT)
	trans_case(D32_FLOAT)
	trans_case(D16_UNORM)
	trans_case(BC1_UNORM)
	trans_case(BC1_UNORM_SRGB)
	trans_case(BC2_UNORM)
	trans_case(BC2_UNORM_SRGB)
	trans_case(BC3_UNORM)
	trans_case(BC3_UNORM_SRGB)
	trans_case(BC4_UNORM)
	trans_case(BC4_SNORM)
	trans_case(BC5_UNORM)
	trans_case(BC5_SNORM)
	trans_case(BC6H_UF16)
	trans_case(BC6H_SF16)
	trans_case(BC7_UNORM)
	trans_case(BC7_UNORM_SRGB)
	default:
		assert("format not supported!");
		break;
	}
	return DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
}

#undef trans_case

#ifdef __cplusplus
} // end extern "C"
#endif