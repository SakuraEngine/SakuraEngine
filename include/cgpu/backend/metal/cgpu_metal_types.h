#pragma once
#include "cgpu_metal.h"
#ifndef __OBJC__
    #include <assert.h>
static_assert(0, "This Header Should Only Be Included By OBJC SOURCES!!!!!");
#endif
#import <MetalKit/MetalKit.h>

// Types
typedef struct CGpuDevice_Metal {
    CGpuDevice super;
    id<MTLDevice> pDevice;
} CGpuDevice_Metal;

typedef struct CGpuAdapter_Metal {
    CGpuAdapter super;
    CGpuDevice_Metal device;

    CGpuAdapterDetail adapter_detail;
} CGpuAdapter_Metal;

typedef struct CGpuInstance_Metal {
    CGpuInstance super;
    CGpuAdapter_Metal adapter;
} CGpuInstance_Metal;

/* clang-format off */
inline static MTLPixelFormat pf_translate_to_metal(const ECGpuPixelFormat fmt)
{
    switch (fmt) {
	case PF_A8_UNORM: 		return MTLPixelFormatA8Unorm;
	case PF_R8_UNORM: 		return MTLPixelFormatR8Unorm;
	case PF_R8_SNORM: 		return MTLPixelFormatR8Snorm;
	case PF_R8_UINT: 		return MTLPixelFormatR8Uint;
	case PF_R8_SINT: 		return MTLPixelFormatR8Sint;
	case PF_R8_SRGB: 		return MTLPixelFormatR8Unorm_sRGB;
	case PF_UNDEFINED: return MTLPixelFormatInvalid;
	case PF_R4G4B4A4_UNORM: return MTLPixelFormatABGR4Unorm;
	case PF_R5G6B5_UNORM: return MTLPixelFormatB5G6R5Unorm;;
	case PF_R5G5B5A1_UNORM: return MTLPixelFormatA1BGR5Unorm;
	case PF_R8G8_UNORM:		return MTLPixelFormatRG8Unorm;
	case PF_R8G8_SNORM:		return MTLPixelFormatRG8Snorm;
	case PF_R8G8_UINT:			return MTLPixelFormatRG8Uint;
	case PF_R8G8_SINT:			return MTLPixelFormatRG8Sint;
	case PF_R8G8_SRGB:			return MTLPixelFormatRG8Unorm_sRGB;
	case PF_R8G8B8A8_UNORM:		return MTLPixelFormatRGBA8Unorm;
	case PF_R8G8B8A8_SNORM:		return MTLPixelFormatRGBA8Snorm;
	case PF_R8G8B8A8_UINT:			return MTLPixelFormatRGBA8Uint;
	case PF_R8G8B8A8_SINT:			return MTLPixelFormatRGBA8Sint;
	case PF_R8G8B8A8_SRGB:			return MTLPixelFormatRGBA8Unorm_sRGB;
	case PF_B8G8R8A8_UNORM: 							return MTLPixelFormatBGRA8Unorm;
	case PF_B8G8R8A8_SRGB:								return MTLPixelFormatBGRA8Unorm_sRGB;
	case PF_B10G10R10A2_UNORM:		return MTLPixelFormatBGR10A2Unorm;
	case PF_R10G10B10A2_UNORM:		return MTLPixelFormatRGB10A2Unorm;
	case PF_R10G10B10A2_UINT:			return MTLPixelFormatRGB10A2Uint;
	case PF_R16_UNORM:			return MTLPixelFormatR16Unorm;
	case PF_R16_SNORM:			return MTLPixelFormatR16Snorm;
	case PF_R16_UINT:			return MTLPixelFormatR16Uint;
	case PF_R16_SINT:			return MTLPixelFormatR16Sint;
	case PF_R16_SFLOAT:		return MTLPixelFormatR16Float;
	case PF_R16G16_UNORM:	return MTLPixelFormatRG16Unorm;
	case PF_R16G16_SNORM:	return MTLPixelFormatRG16Snorm;
	case PF_R16G16_UINT:		return MTLPixelFormatRG16Uint;
	case PF_R16G16_SINT:		return MTLPixelFormatRG16Sint;
	case PF_R16G16_SFLOAT:	return MTLPixelFormatRG16Float;
	case PF_R16G16B16A16_UNORM:	return MTLPixelFormatRGBA16Unorm;
	case PF_R16G16B16A16_SNORM:	return MTLPixelFormatRGBA16Snorm;
	case PF_R16G16B16A16_UINT:		return MTLPixelFormatRGBA16Uint;
	case PF_R16G16B16A16_SINT:		return MTLPixelFormatRGBA16Sint;
	case PF_R16G16B16A16_SFLOAT:	return MTLPixelFormatRGBA16Float;
	case PF_R32_UINT:						return MTLPixelFormatR32Uint;
	case PF_R32_SINT:						return MTLPixelFormatR32Sint;
	case PF_R32_SFLOAT:					return MTLPixelFormatR32Float;
	case PF_R32G32_UINT:					return MTLPixelFormatRG32Uint;
	case PF_R32G32_SINT:					return MTLPixelFormatRG32Sint;
	case PF_R32G32_SFLOAT:				return MTLPixelFormatRG32Float;
	case PF_R32G32B32A32_UINT:		return MTLPixelFormatRGBA32Uint;
	case PF_R32G32B32A32_SINT:		return MTLPixelFormatRGBA32Sint;
	case PF_R32G32B32A32_SFLOAT:	return MTLPixelFormatRGBA32Float;
	case PF_B10G11R11_UFLOAT: return MTLPixelFormatRG11B10Float;
	case PF_E5B9G9R9_UFLOAT:	return MTLPixelFormatRGB9E5Float;
	case PF_D16_UNORM:								return MTLPixelFormatDepth16Unorm;
	case PF_X8_D24_UNORM:			return MTLPixelFormatDepth24Unorm_Stencil8;
	case PF_D32_SFLOAT:							return MTLPixelFormatDepth32Float;
	case PF_S8_UINT:									return MTLPixelFormatStencil8;
	case PF_D24_UNORM_S8_UINT:				return MTLPixelFormatDepth24Unorm_Stencil8;
	case PF_D32_SFLOAT_S8_UINT:			return MTLPixelFormatDepth32Float_Stencil8;
	case PF_DXBC1_RGB_UNORM:			return MTLPixelFormatBC1_RGBA;
	case PF_DXBC1_RGB_SRGB:			return MTLPixelFormatBC1_RGBA_sRGB;
	case PF_DXBC1_RGBA_UNORM:		return MTLPixelFormatBC1_RGBA;
	case PF_DXBC1_RGBA_SRGB:			return MTLPixelFormatBC1_RGBA_sRGB;
	case PF_DXBC2_UNORM:					return MTLPixelFormatBC2_RGBA;
	case PF_DXBC2_SRGB:					return MTLPixelFormatBC2_RGBA_sRGB;
	case PF_DXBC3_UNORM:					return MTLPixelFormatBC3_RGBA;
	case PF_DXBC3_SRGB:					return MTLPixelFormatBC3_RGBA_sRGB;
	case PF_DXBC4_UNORM:					return MTLPixelFormatBC4_RUnorm;
	case PF_DXBC4_SNORM:					return MTLPixelFormatBC4_RSnorm;
	case PF_DXBC5_UNORM:					return MTLPixelFormatBC5_RGUnorm;
	case PF_DXBC5_SNORM:					return MTLPixelFormatBC5_RGSnorm;
	case PF_DXBC6H_UFLOAT:				return MTLPixelFormatBC6H_RGBUfloat;
	case PF_DXBC6H_SFLOAT:				return MTLPixelFormatBC6H_RGBFloat;
	case PF_DXBC7_UNORM:					return MTLPixelFormatBC7_RGBAUnorm;
	case PF_DXBC7_SRGB:					return MTLPixelFormatBC7_RGBAUnorm_sRGB;
	case PF_PVRTC1_2BPP_UNORM:		return MTLPixelFormatPVRTC_RGBA_2BPP;
	case PF_PVRTC1_4BPP_UNORM:		return MTLPixelFormatPVRTC_RGBA_4BPP;
	case PF_PVRTC1_2BPP_SRGB:		return MTLPixelFormatPVRTC_RGBA_2BPP_sRGB;
	case PF_PVRTC1_4BPP_SRGB:		return MTLPixelFormatPVRTC_RGBA_4BPP_sRGB;
	case PF_ETC2_R8G8B8_UNORM:	return MTLPixelFormatETC2_RGB8;
	case PF_ETC2_R8G8B8A1_UNORM:return MTLPixelFormatETC2_RGB8A1;
	case PF_ETC2_R8G8B8A8_UNORM: return MTLPixelFormatEAC_RGBA8;
	case PF_ETC2_R8G8B8_SRGB:		return MTLPixelFormatETC2_RGB8_sRGB;
	case PF_ETC2_R8G8B8A1_SRGB:	return MTLPixelFormatETC2_RGB8A1_sRGB;
	case PF_ETC2_R8G8B8A8_SRGB:	return MTLPixelFormatEAC_RGBA8_sRGB;
	case PF_ETC2_EAC_R11_UNORM:				return MTLPixelFormatEAC_R11Unorm;
	case PF_ETC2_EAC_R11G11_UNORM:		return MTLPixelFormatEAC_RG11Unorm;
	case PF_ETC2_EAC_R11_SNORM:				return MTLPixelFormatEAC_R11Snorm;
	case PF_ETC2_EAC_R11G11_SNORM:		return MTLPixelFormatEAC_RG11Snorm;
	case PF_ASTC_4x4_UNORM:			return MTLPixelFormatASTC_4x4_LDR;
	case PF_ASTC_4x4_SRGB:				return MTLPixelFormatASTC_4x4_sRGB;
	case PF_ASTC_5x4_UNORM:			return MTLPixelFormatASTC_5x4_LDR;
	case PF_ASTC_5x4_SRGB:				return MTLPixelFormatASTC_5x4_sRGB;
	case PF_ASTC_5x5_UNORM:			return MTLPixelFormatASTC_5x5_LDR;
	case PF_ASTC_5x5_SRGB:				return MTLPixelFormatASTC_5x5_sRGB;
	case PF_ASTC_6x5_UNORM:			return MTLPixelFormatASTC_6x5_LDR;
	case PF_ASTC_6x5_SRGB:				return MTLPixelFormatASTC_6x5_sRGB;
	case PF_ASTC_6x6_UNORM:			return MTLPixelFormatASTC_6x6_LDR;
	case PF_ASTC_6x6_SRGB:				return MTLPixelFormatASTC_6x6_sRGB;
	case PF_ASTC_8x5_UNORM:			return MTLPixelFormatASTC_8x5_LDR;
	case PF_ASTC_8x5_SRGB:				return MTLPixelFormatASTC_8x5_sRGB;
	case PF_ASTC_8x6_UNORM:			return MTLPixelFormatASTC_8x6_LDR;
	case PF_ASTC_8x6_SRGB:				return MTLPixelFormatASTC_8x6_sRGB;
	case PF_ASTC_8x8_UNORM:			return MTLPixelFormatASTC_8x8_LDR;
	case PF_ASTC_8x8_SRGB:				return MTLPixelFormatASTC_8x8_sRGB;
	case PF_ASTC_10x5_UNORM:			return MTLPixelFormatASTC_10x5_LDR;
	case PF_ASTC_10x5_SRGB:			return MTLPixelFormatASTC_10x5_sRGB;
	case PF_ASTC_10x6_UNORM:			return MTLPixelFormatASTC_10x6_LDR;
	case PF_ASTC_10x6_SRGB:			return MTLPixelFormatASTC_10x6_sRGB;
	case PF_ASTC_10x8_UNORM:			return MTLPixelFormatASTC_10x8_LDR;
	case PF_ASTC_10x8_SRGB:			return MTLPixelFormatASTC_10x8_sRGB;
	case PF_ASTC_10x10_UNORM:		return MTLPixelFormatASTC_10x10_LDR;
	case PF_ASTC_10x10_SRGB:			return MTLPixelFormatASTC_10x10_sRGB;
	case PF_ASTC_12x10_UNORM:		return MTLPixelFormatASTC_12x10_LDR;
	case PF_ASTC_12x10_SRGB:			return MTLPixelFormatASTC_12x10_sRGB;
	case PF_ASTC_12x12_UNORM:		return MTLPixelFormatASTC_12x12_LDR;
	case PF_ASTC_12x12_SRGB:			return MTLPixelFormatASTC_12x12_sRGB;

	default: return MTLPixelFormatInvalid;
	}

	return MTLPixelFormatInvalid;
}

inline static bool MetalFormatOkayOnMac(MTLPixelFormat fmt)
{
    switch (fmt)
    {
        case MTLPixelFormatA8Unorm:
        case MTLPixelFormatR8Unorm:
        case MTLPixelFormatR8Snorm:
        case MTLPixelFormatR8Uint:
        case MTLPixelFormatR8Sint:
        case MTLPixelFormatR16Unorm:
        case MTLPixelFormatR16Snorm:
        case MTLPixelFormatR16Uint:
        case MTLPixelFormatR16Sint:
        case MTLPixelFormatR16Float:
        case MTLPixelFormatRG8Unorm:
        case MTLPixelFormatRG8Snorm:
        case MTLPixelFormatRG8Uint:
        case MTLPixelFormatRG8Sint:
        case MTLPixelFormatR32Uint:
        case MTLPixelFormatR32Sint:
        case MTLPixelFormatR32Float:
        case MTLPixelFormatRG16Unorm:
        case MTLPixelFormatRG16Snorm:
        case MTLPixelFormatRG16Uint:
        case MTLPixelFormatRG16Sint:
        case MTLPixelFormatRG16Float:
        case MTLPixelFormatRGBA8Unorm:
        case MTLPixelFormatRGBA8Unorm_sRGB:
        case MTLPixelFormatRGBA8Snorm:
        case MTLPixelFormatRGBA8Uint:
        case MTLPixelFormatRGBA8Sint:
        case MTLPixelFormatBGRA8Unorm:
        case MTLPixelFormatBGRA8Unorm_sRGB:
        case MTLPixelFormatRGB10A2Unorm:
        case MTLPixelFormatRGB10A2Uint:
        case MTLPixelFormatRG11B10Float:
        case MTLPixelFormatRGB9E5Float:
        case MTLPixelFormatBGR10A2Unorm:
        case MTLPixelFormatRG32Uint:
        case MTLPixelFormatRG32Sint:
        case MTLPixelFormatRG32Float:
        case MTLPixelFormatRGBA16Unorm:
        case MTLPixelFormatRGBA16Snorm:
        case MTLPixelFormatRGBA16Uint:
        case MTLPixelFormatRGBA16Sint:
        case MTLPixelFormatRGBA16Float:
        case MTLPixelFormatRGBA32Uint:
        case MTLPixelFormatRGBA32Sint:
        case MTLPixelFormatRGBA32Float:
        case MTLPixelFormatBC1_RGBA:
        case MTLPixelFormatBC1_RGBA_sRGB:
        case MTLPixelFormatBC2_RGBA:
        case MTLPixelFormatBC2_RGBA_sRGB:
        case MTLPixelFormatBC3_RGBA:
        case MTLPixelFormatBC3_RGBA_sRGB:
        case MTLPixelFormatBC4_RUnorm:
        case MTLPixelFormatBC4_RSnorm:
        case MTLPixelFormatBC5_RGUnorm:
        case MTLPixelFormatBC5_RGSnorm:
        case MTLPixelFormatBC6H_RGBFloat:
        case MTLPixelFormatBC6H_RGBUfloat:
        case MTLPixelFormatBC7_RGBAUnorm:
        case MTLPixelFormatBC7_RGBAUnorm_sRGB:
        case MTLPixelFormatGBGR422:
        case MTLPixelFormatBGRG422:
        case MTLPixelFormatDepth16Unorm:
        case MTLPixelFormatDepth32Float:
        case MTLPixelFormatStencil8:
        case MTLPixelFormatDepth24Unorm_Stencil8:
        case MTLPixelFormatDepth32Float_Stencil8:
        case MTLPixelFormatX32_Stencil8:
        case MTLPixelFormatX24_Stencil8:
            return true;

        case MTLPixelFormatBGRA10_XR:
        case MTLPixelFormatBGRA10_XR_sRGB:
        case MTLPixelFormatBGR10_XR:
        case MTLPixelFormatBGR10_XR_sRGB:
        case MTLPixelFormatB5G6R5Unorm:
        case MTLPixelFormatA1BGR5Unorm:
        case MTLPixelFormatABGR4Unorm:
        case MTLPixelFormatBGR5A1Unorm:
        case MTLPixelFormatR8Unorm_sRGB:
        case MTLPixelFormatRG8Unorm_sRGB:
        case MTLPixelFormatPVRTC_RGB_2BPP:
        case MTLPixelFormatPVRTC_RGB_2BPP_sRGB:
        case MTLPixelFormatPVRTC_RGB_4BPP:
        case MTLPixelFormatPVRTC_RGB_4BPP_sRGB:
        case MTLPixelFormatPVRTC_RGBA_2BPP:
        case MTLPixelFormatPVRTC_RGBA_2BPP_sRGB:
        case MTLPixelFormatPVRTC_RGBA_4BPP:
        case MTLPixelFormatPVRTC_RGBA_4BPP_sRGB:
        case MTLPixelFormatEAC_R11Unorm:
        case MTLPixelFormatEAC_R11Snorm:
        case MTLPixelFormatEAC_RG11Unorm:
        case MTLPixelFormatEAC_RG11Snorm:
        case MTLPixelFormatEAC_RGBA8:
        case MTLPixelFormatEAC_RGBA8_sRGB:
        case MTLPixelFormatETC2_RGB8:
        case MTLPixelFormatETC2_RGB8_sRGB:
        case MTLPixelFormatETC2_RGB8A1:
        case MTLPixelFormatETC2_RGB8A1_sRGB:
        case MTLPixelFormatASTC_4x4_sRGB:
        case MTLPixelFormatASTC_5x4_sRGB:
        case MTLPixelFormatASTC_5x5_sRGB:
        case MTLPixelFormatASTC_6x5_sRGB:
        case MTLPixelFormatASTC_6x6_sRGB:
        case MTLPixelFormatASTC_8x5_sRGB:
        case MTLPixelFormatASTC_8x6_sRGB:
        case MTLPixelFormatASTC_8x8_sRGB:
        case MTLPixelFormatASTC_10x5_sRGB:
        case MTLPixelFormatASTC_10x6_sRGB:
        case MTLPixelFormatASTC_10x8_sRGB:
        case MTLPixelFormatASTC_10x10_sRGB:
        case MTLPixelFormatASTC_12x10_sRGB:
        case MTLPixelFormatASTC_12x12_sRGB:
        case MTLPixelFormatASTC_4x4_LDR:
        case MTLPixelFormatASTC_5x4_LDR:
        case MTLPixelFormatASTC_5x5_LDR:
        case MTLPixelFormatASTC_6x5_LDR:
        case MTLPixelFormatASTC_6x6_LDR:
        case MTLPixelFormatASTC_8x5_LDR:
        case MTLPixelFormatASTC_8x6_LDR:
        case MTLPixelFormatASTC_8x8_LDR:
        case MTLPixelFormatASTC_10x5_LDR:
        case MTLPixelFormatASTC_10x6_LDR:
        case MTLPixelFormatASTC_10x8_LDR:
        case MTLPixelFormatASTC_10x10_LDR:
        case MTLPixelFormatASTC_12x10_LDR:
        case MTLPixelFormatASTC_12x12_LDR:
        case MTLPixelFormatInvalid: 
        default:
            return false;
    }
    return false;
}

inline static bool MetalFormatOkayOnIOS(MTLPixelFormat fmt) {
	switch(fmt) 
    {
	    case MTLPixelFormatA8Unorm:
	    case MTLPixelFormatR8Unorm:
	    case MTLPixelFormatR8Snorm:
	    case MTLPixelFormatR8Uint:
	    case MTLPixelFormatR8Sint:
	    case MTLPixelFormatR16Unorm:
	    case MTLPixelFormatR16Snorm:
	    case MTLPixelFormatR16Uint:
	    case MTLPixelFormatR16Sint:
	    case MTLPixelFormatR16Float:
	    case MTLPixelFormatRG8Unorm:
	    case MTLPixelFormatRG8Snorm:
	    case MTLPixelFormatRG8Uint:
	    case MTLPixelFormatRG8Sint:
	    case MTLPixelFormatR32Uint:
	    case MTLPixelFormatR32Sint:
	    case MTLPixelFormatR32Float:
	    case MTLPixelFormatRG16Unorm:
	    case MTLPixelFormatRG16Snorm:
	    case MTLPixelFormatRG16Uint:
	    case MTLPixelFormatRG16Sint:
	    case MTLPixelFormatRG16Float:
	    case MTLPixelFormatRGBA8Unorm:
	    case MTLPixelFormatRGBA8Unorm_sRGB:
	    case MTLPixelFormatRGBA8Snorm:
	    case MTLPixelFormatRGBA8Uint:
	    case MTLPixelFormatRGBA8Sint:
	    case MTLPixelFormatBGRA8Unorm:
	    case MTLPixelFormatBGRA8Unorm_sRGB:
	    case MTLPixelFormatRGB10A2Unorm:
	    case MTLPixelFormatRGB10A2Uint:
	    case MTLPixelFormatRG11B10Float:
	    case MTLPixelFormatRGB9E5Float:
	    case MTLPixelFormatBGR10A2Unorm:
	    case MTLPixelFormatRG32Uint:
	    case MTLPixelFormatRG32Sint:
	    case MTLPixelFormatRG32Float:
	    case MTLPixelFormatRGBA16Unorm:
	    case MTLPixelFormatRGBA16Snorm:
	    case MTLPixelFormatRGBA16Uint:
	    case MTLPixelFormatRGBA16Sint:
	    case MTLPixelFormatRGBA16Float:
	    case MTLPixelFormatRGBA32Uint:
	    case MTLPixelFormatRGBA32Sint:
	    case MTLPixelFormatRGBA32Float:
	    case MTLPixelFormatGBGR422:
	    case MTLPixelFormatBGRG422:
	    case MTLPixelFormatDepth32Float:
	    case MTLPixelFormatStencil8:
	    case MTLPixelFormatDepth32Float_Stencil8:
	    case MTLPixelFormatX32_Stencil8:
	    case MTLPixelFormatBGRA10_XR:
	    case MTLPixelFormatBGRA10_XR_sRGB:
	    case MTLPixelFormatBGR10_XR:
	    case MTLPixelFormatBGR10_XR_sRGB:
	    case MTLPixelFormatPVRTC_RGB_2BPP:
	    case MTLPixelFormatPVRTC_RGB_2BPP_sRGB:
	    case MTLPixelFormatPVRTC_RGB_4BPP:
	    case MTLPixelFormatPVRTC_RGB_4BPP_sRGB:
	    case MTLPixelFormatPVRTC_RGBA_2BPP:
	    case MTLPixelFormatPVRTC_RGBA_2BPP_sRGB:
	    case MTLPixelFormatPVRTC_RGBA_4BPP:
	    case MTLPixelFormatPVRTC_RGBA_4BPP_sRGB:
	    case MTLPixelFormatEAC_R11Unorm:
	    case MTLPixelFormatEAC_R11Snorm:
	    case MTLPixelFormatEAC_RG11Unorm:
	    case MTLPixelFormatEAC_RG11Snorm:
	    case MTLPixelFormatEAC_RGBA8:
	    case MTLPixelFormatEAC_RGBA8_sRGB:
	    case MTLPixelFormatETC2_RGB8:
	    case MTLPixelFormatETC2_RGB8_sRGB:
	    case MTLPixelFormatETC2_RGB8A1:
	    case MTLPixelFormatETC2_RGB8A1_sRGB:
	    case MTLPixelFormatASTC_4x4_sRGB:
	    case MTLPixelFormatASTC_5x4_sRGB:
	    case MTLPixelFormatASTC_5x5_sRGB:
	    case MTLPixelFormatASTC_6x5_sRGB:
	    case MTLPixelFormatASTC_6x6_sRGB:
	    case MTLPixelFormatASTC_8x5_sRGB:
	    case MTLPixelFormatASTC_8x6_sRGB:
	    case MTLPixelFormatASTC_8x8_sRGB:
	    case MTLPixelFormatASTC_10x5_sRGB:
	    case MTLPixelFormatASTC_10x6_sRGB:
	    case MTLPixelFormatASTC_10x8_sRGB:
	    case MTLPixelFormatASTC_10x10_sRGB:
	    case MTLPixelFormatASTC_12x10_sRGB:
	    case MTLPixelFormatASTC_12x12_sRGB:
	    case MTLPixelFormatASTC_4x4_LDR:
	    case MTLPixelFormatASTC_5x4_LDR:
	    case MTLPixelFormatASTC_5x5_LDR:
	    case MTLPixelFormatASTC_6x5_LDR:
	    case MTLPixelFormatASTC_6x6_LDR:
	    case MTLPixelFormatASTC_8x5_LDR:
	    case MTLPixelFormatASTC_8x6_LDR:
	    case MTLPixelFormatASTC_8x8_LDR:
	    case MTLPixelFormatASTC_10x5_LDR:
	    case MTLPixelFormatASTC_10x6_LDR:
	    case MTLPixelFormatASTC_10x8_LDR:
	    case MTLPixelFormatASTC_10x10_LDR:
	    case MTLPixelFormatASTC_12x10_LDR:
	    case MTLPixelFormatASTC_12x12_LDR:
	    case MTLPixelFormatB5G6R5Unorm:
	    case MTLPixelFormatA1BGR5Unorm:
	    case MTLPixelFormatABGR4Unorm:
	    case MTLPixelFormatBGR5A1Unorm:
	    case MTLPixelFormatR8Unorm_sRGB:
	    case MTLPixelFormatRG8Unorm_sRGB:
		    return true;

	    case MTLPixelFormatDepth16Unorm:
	    case MTLPixelFormatDepth24Unorm_Stencil8:
	    case MTLPixelFormatX24_Stencil8:
	    case MTLPixelFormatBC1_RGBA:
	    case MTLPixelFormatBC1_RGBA_sRGB:
	    case MTLPixelFormatBC2_RGBA:
	    case MTLPixelFormatBC2_RGBA_sRGB:
	    case MTLPixelFormatBC3_RGBA:
	    case MTLPixelFormatBC3_RGBA_sRGB:
	    case MTLPixelFormatBC4_RUnorm:
	    case MTLPixelFormatBC4_RSnorm:
	    case MTLPixelFormatBC5_RGUnorm:
	    case MTLPixelFormatBC5_RGSnorm:
	    case MTLPixelFormatBC6H_RGBFloat:
	    case MTLPixelFormatBC6H_RGBUfloat:
	    case MTLPixelFormatBC7_RGBAUnorm:
	    case MTLPixelFormatBC7_RGBAUnorm_sRGB:
	    case MTLPixelFormatInvalid: 
        default:
            return false;
	}
    return false;
}
