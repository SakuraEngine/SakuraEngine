#pragma once
#include "cgpu_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// enums
typedef enum ECGpuNvAPI_Status
{
    CGPU_NVAPI_OK = 0, //!< Success. Request is completed.
    CGPU_NVAPI_NONE = 1,
    CGPU_NVAPI_ERROR = -1, //!< Generic error
    CGPU_NVAPI_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuNvAPI_Status;

typedef enum ECGpuAGSReturnCode
{
    CGPU_AGS_SUCCESS,                 ///< Successful function call
    CGPU_AGS_FAILURE,                 ///< Failed to complete call for some unspecified reason
    CGPU_AGS_INVALID_ARGS,            ///< Invalid arguments into the function
    CGPU_AGS_OUT_OF_MEMORY,           ///< Out of memory when allocating space internally
    CGPU_AGS_MISSING_D3D_DLL,         ///< Returned when a D3D dll fails to load
    CGPU_AGS_LEGACY_DRIVER,           ///< Returned if a feature is not present in the installed driver
    CGPU_AGS_NO_AMD_DRIVER_INSTALLED, ///< Returned if the AMD GPU driver does not appear to be installed
    CGPU_AGS_EXTENSION_NOT_SUPPORTED, ///< Returned if the driver does not support the requested driver extension
    CGPU_AGS_ADL_FAILURE,             ///< Failure in ADL (the AMD Display Library)
    CGPU_AGS_DX_FAILURE,              ///< Failure from DirectX runtime
    CGPU_AGS_NONE,
    CGPU_AGS_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuAGSReturnCode;

// Format
typedef enum ECGpuFormat
{
    PF_UNDEFINED = 0,
    PF_R1_UNORM = 1,
    PF_R2_UNORM = 2,
    PF_R4_UNORM = 3,
    PF_R4G4_UNORM = 4,
    PF_G4R4_UNORM = 5,
    PF_A8_UNORM = 6,
    PF_R8_UNORM = 7,
    PF_R8_SNORM = 8,
    PF_R8_UINT = 9,
    PF_R8_SINT = 10,
    PF_R8_SRGB = 11,
    PF_B2G3R3_UNORM = 12,
    PF_R4G4B4A4_UNORM = 13,
    PF_R4G4B4X4_UNORM = 14,
    PF_B4G4R4A4_UNORM = 15,
    PF_B4G4R4X4_UNORM = 16,
    PF_A4R4G4B4_UNORM = 17,
    PF_X4R4G4B4_UNORM = 18,
    PF_A4B4G4R4_UNORM = 19,
    PF_X4B4G4R4_UNORM = 20,
    PF_R5G6B5_UNORM = 21,
    PF_B5G6R5_UNORM = 22,
    PF_R5G5B5A1_UNORM = 23,
    PF_B5G5R5A1_UNORM = 24,
    PF_A1B5G5R5_UNORM = 25,
    PF_A1R5G5B5_UNORM = 26,
    PF_R5G5B5X1_UNORM = 27,
    PF_B5G5R5X1_UNORM = 28,
    PF_X1R5G5B5_UNORM = 29,
    PF_X1B5G5R5_UNORM = 30,
    PF_B2G3R3A8_UNORM = 31,
    PF_R8G8_UNORM = 32,
    PF_R8G8_SNORM = 33,
    PF_G8R8_UNORM = 34,
    PF_G8R8_SNORM = 35,
    PF_R8G8_UINT = 36,
    PF_R8G8_SINT = 37,
    PF_R8G8_SRGB = 38,
    PF_R16_UNORM = 39,
    PF_R16_SNORM = 40,
    PF_R16_UINT = 41,
    PF_R16_SINT = 42,
    PF_R16_SFLOAT = 43,
    PF_R16_SBFLOAT = 44,
    PF_R8G8B8_UNORM = 45,
    PF_R8G8B8_SNORM = 46,
    PF_R8G8B8_UINT = 47,
    PF_R8G8B8_SINT = 48,
    PF_R8G8B8_SRGB = 49,
    PF_B8G8R8_UNORM = 50,
    PF_B8G8R8_SNORM = 51,
    PF_B8G8R8_UINT = 52,
    PF_B8G8R8_SINT = 53,
    PF_B8G8R8_SRGB = 54,
    PF_R8G8B8A8_UNORM = 55,
    PF_R8G8B8A8_SNORM = 56,
    PF_R8G8B8A8_UINT = 57,
    PF_R8G8B8A8_SINT = 58,
    PF_R8G8B8A8_SRGB = 59,
    PF_B8G8R8A8_UNORM = 60,
    PF_B8G8R8A8_SNORM = 61,
    PF_B8G8R8A8_UINT = 62,
    PF_B8G8R8A8_SINT = 63,
    PF_B8G8R8A8_SRGB = 64,
    PF_R8G8B8X8_UNORM = 65,
    PF_B8G8R8X8_UNORM = 66,
    PF_R16G16_UNORM = 67,
    PF_G16R16_UNORM = 68,
    PF_R16G16_SNORM = 69,
    PF_G16R16_SNORM = 70,
    PF_R16G16_UINT = 71,
    PF_R16G16_SINT = 72,
    PF_R16G16_SFLOAT = 73,
    PF_R16G16_SBFLOAT = 74,
    PF_R32_UINT = 75,
    PF_R32_SINT = 76,
    PF_R32_SFLOAT = 77,
    PF_A2R10G10B10_UNORM = 78,
    PF_A2R10G10B10_UINT = 79,
    PF_A2R10G10B10_SNORM = 80,
    PF_A2R10G10B10_SINT = 81,
    PF_A2B10G10R10_UNORM = 82,
    PF_A2B10G10R10_UINT = 83,
    PF_A2B10G10R10_SNORM = 84,
    PF_A2B10G10R10_SINT = 85,
    PF_R10G10B10A2_UNORM = 86,
    PF_R10G10B10A2_UINT = 87,
    PF_R10G10B10A2_SNORM = 88,
    PF_R10G10B10A2_SINT = 89,
    PF_B10G10R10A2_UNORM = 90,
    PF_B10G10R10A2_UINT = 91,
    PF_B10G10R10A2_SNORM = 92,
    PF_B10G10R10A2_SINT = 93,
    PF_B10G11R11_UFLOAT = 94,
    PF_E5B9G9R9_UFLOAT = 95,
    PF_R16G16B16_UNORM = 96,
    PF_R16G16B16_SNORM = 97,
    PF_R16G16B16_UINT = 98,
    PF_R16G16B16_SINT = 99,
    PF_R16G16B16_SFLOAT = 100,
    PF_R16G16B16_SBFLOAT = 101,
    PF_R16G16B16A16_UNORM = 102,
    PF_R16G16B16A16_SNORM = 103,
    PF_R16G16B16A16_UINT = 104,
    PF_R16G16B16A16_SINT = 105,
    PF_R16G16B16A16_SFLOAT = 106,
    PF_R16G16B16A16_SBFLOAT = 107,
    PF_R32G32_UINT = 108,
    PF_R32G32_SINT = 109,
    PF_R32G32_SFLOAT = 110,
    PF_R32G32B32_UINT = 111,
    PF_R32G32B32_SINT = 112,
    PF_R32G32B32_SFLOAT = 113,
    PF_R32G32B32A32_UINT = 114,
    PF_R32G32B32A32_SINT = 115,
    PF_R32G32B32A32_SFLOAT = 116,
    PF_R64_UINT = 117,
    PF_R64_SINT = 118,
    PF_R64_SFLOAT = 119,
    PF_R64G64_UINT = 120,
    PF_R64G64_SINT = 121,
    PF_R64G64_SFLOAT = 122,
    PF_R64G64B64_UINT = 123,
    PF_R64G64B64_SINT = 124,
    PF_R64G64B64_SFLOAT = 125,
    PF_R64G64B64A64_UINT = 126,
    PF_R64G64B64A64_SINT = 127,
    PF_R64G64B64A64_SFLOAT = 128,
    PF_D16_UNORM = 129,
    PF_X8_D24_UNORM = 130,
    PF_D32_SFLOAT = 131,
    PF_S8_UINT = 132,
    PF_D16_UNORM_S8_UINT = 133,
    PF_D24_UNORM_S8_UINT = 134,
    PF_D32_SFLOAT_S8_UINT = 135,
    PF_DXBC1_RGB_UNORM = 136,
    PF_DXBC1_RGB_SRGB = 137,
    PF_DXBC1_RGBA_UNORM = 138,
    PF_DXBC1_RGBA_SRGB = 139,
    PF_DXBC2_UNORM = 140,
    PF_DXBC2_SRGB = 141,
    PF_DXBC3_UNORM = 142,
    PF_DXBC3_SRGB = 143,
    PF_DXBC4_UNORM = 144,
    PF_DXBC4_SNORM = 145,
    PF_DXBC5_UNORM = 146,
    PF_DXBC5_SNORM = 147,
    PF_DXBC6H_UFLOAT = 148,
    PF_DXBC6H_SFLOAT = 149,
    PF_DXBC7_UNORM = 150,
    PF_DXBC7_SRGB = 151,
    PF_PVRTC1_2BPP_UNORM = 152,
    PF_PVRTC1_4BPP_UNORM = 153,
    PF_PVRTC2_2BPP_UNORM = 154,
    PF_PVRTC2_4BPP_UNORM = 155,
    PF_PVRTC1_2BPP_SRGB = 156,
    PF_PVRTC1_4BPP_SRGB = 157,
    PF_PVRTC2_2BPP_SRGB = 158,
    PF_PVRTC2_4BPP_SRGB = 159,
    PF_ETC2_R8G8B8_UNORM = 160,
    PF_ETC2_R8G8B8_SRGB = 161,
    PF_ETC2_R8G8B8A1_UNORM = 162,
    PF_ETC2_R8G8B8A1_SRGB = 163,
    PF_ETC2_R8G8B8A8_UNORM = 164,
    PF_ETC2_R8G8B8A8_SRGB = 165,
    PF_ETC2_EAC_R11_UNORM = 166,
    PF_ETC2_EAC_R11_SNORM = 167,
    PF_ETC2_EAC_R11G11_UNORM = 168,
    PF_ETC2_EAC_R11G11_SNORM = 169,
    PF_ASTC_4x4_UNORM = 170,
    PF_ASTC_4x4_SRGB = 171,
    PF_ASTC_5x4_UNORM = 172,
    PF_ASTC_5x4_SRGB = 173,
    PF_ASTC_5x5_UNORM = 174,
    PF_ASTC_5x5_SRGB = 175,
    PF_ASTC_6x5_UNORM = 176,
    PF_ASTC_6x5_SRGB = 177,
    PF_ASTC_6x6_UNORM = 178,
    PF_ASTC_6x6_SRGB = 179,
    PF_ASTC_8x5_UNORM = 180,
    PF_ASTC_8x5_SRGB = 181,
    PF_ASTC_8x6_UNORM = 182,
    PF_ASTC_8x6_SRGB = 183,
    PF_ASTC_8x8_UNORM = 184,
    PF_ASTC_8x8_SRGB = 185,
    PF_ASTC_10x5_UNORM = 186,
    PF_ASTC_10x5_SRGB = 187,
    PF_ASTC_10x6_UNORM = 188,
    PF_ASTC_10x6_SRGB = 189,
    PF_ASTC_10x8_UNORM = 190,
    PF_ASTC_10x8_SRGB = 191,
    PF_ASTC_10x10_UNORM = 192,
    PF_ASTC_10x10_SRGB = 193,
    PF_ASTC_12x10_UNORM = 194,
    PF_ASTC_12x10_SRGB = 195,
    PF_ASTC_12x12_UNORM = 196,
    PF_ASTC_12x12_SRGB = 197,
    PF_CLUT_P4 = 198,
    PF_CLUT_P4A4 = 199,
    PF_CLUT_P8 = 200,
    PF_CLUT_P8A8 = 201,
    PF_R4G4B4A4_UNORM_PACK16 = 202,
    PF_B4G4R4A4_UNORM_PACK16 = 203,
    PF_R5G6B5_UNORM_PACK16 = 204,
    PF_B5G6R5_UNORM_PACK16 = 205,
    PF_R5G5B5A1_UNORM_PACK16 = 206,
    PF_B5G5R5A1_UNORM_PACK16 = 207,
    PF_A1R5G5B5_UNORM_PACK16 = 208,
    PF_G16B16G16R16_422_UNORM = 209,
    PF_B16G16R16G16_422_UNORM = 210,
    PF_R12X4G12X4B12X4A12X4_UNORM_4PACK16 = 211,
    PF_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16 = 212,
    PF_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16 = 213,
    PF_R10X6G10X6B10X6A10X6_UNORM_4PACK16 = 214,
    PF_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16 = 215,
    PF_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16 = 216,
    PF_G8B8G8R8_422_UNORM = 217,
    PF_B8G8R8G8_422_UNORM = 218,
    PF_G8_B8_R8_3PLANE_420_UNORM = 219,
    PF_G8_B8R8_2PLANE_420_UNORM = 220,
    PF_G8_B8_R8_3PLANE_422_UNORM = 221,
    PF_G8_B8R8_2PLANE_422_UNORM = 222,
    PF_G8_B8_R8_3PLANE_444_UNORM = 223,
    PF_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 224,
    PF_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 225,
    PF_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 226,
    PF_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16 = 227,
    PF_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16 = 228,
    PF_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 229,
    PF_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 230,
    PF_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 231,
    PF_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16 = 232,
    PF_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16 = 233,
    PF_G16_B16_R16_3PLANE_420_UNORM = 234,
    PF_G16_B16_R16_3PLANE_422_UNORM = 235,
    PF_G16_B16_R16_3PLANE_444_UNORM = 236,
    PF_G16_B16R16_2PLANE_420_UNORM = 237,
    PF_G16_B16R16_2PLANE_422_UNORM = 238,
    PF_COUNT = PF_G16_B16R16_2PLANE_422_UNORM + 1,
    PF_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuFormat;

typedef enum ECGpuSlotMaskBit
{
    CGPU_SLOT_0 = 0x1,
    CGPU_SLOT_1 = 0x2,
    CGPU_SLOT_2 = 0x4,
    CGPU_SLOT_3 = 0x8,
    CGPU_SLOT_4 = 0x10,
    CGPU_SLOT_5 = 0x20,
    CGPU_SLOT_6 = 0x40,
    CGPU_SLOT_7 = 0x80
} ECGpuSlotMaskBit;
typedef uint32_t ECGpuSlotMask;

typedef enum ECGpuFilterType
{
    FILTER_TYPE_NEAREST = 0,
    FILTER_TYPE_LINEAR,
    FILTER_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuFilterType;

typedef enum ECGpuAddressMode
{
    ADDRESS_MODE_MIRROR,
    ADDRESS_MODE_REPEAT,
    ADDRESS_MODE_CLAMP_TO_EDGE,
    ADDRESS_MODE_CLAMP_TO_BORDER,
    ADDRESS_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuAddressMode;

typedef enum ECGpuMipMapMode
{
    MIPMAP_MODE_NEAREST = 0,
    MIPMAP_MODE_LINEAR,
    MIPMAP_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuMipMapMode;

typedef enum ECGpuLoadAction
{
    LOAD_ACTION_DONTCARE,
    LOAD_ACTION_LOAD,
    LOAD_ACTION_CLEAR,
    LOAD_ACTION_COUNT,
    LOAD_ACTION_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuLoadAction;

typedef enum ECGpuStoreAction
{
    STORE_ACTION_STORE,
    STORE_ACTION_DISCARD,
    STORE_ACTION_COUNT,
    STORE_ACTION_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuStoreAction;

typedef enum ECGpuPrimitiveTopology
{
    PRIM_TOPO_POINT_LIST = 0,
    PRIM_TOPO_LINE_LIST,
    PRIM_TOPO_LINE_STRIP,
    PRIM_TOPO_TRI_LIST,
    PRIM_TOPO_TRI_STRIP,
    PRIM_TOPO_PATCH_LIST,
    PRIM_TOPO_COUNT,
    PRIM_TOPO_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuPrimitiveTopology;

typedef enum ECGpuBlendConstant
{
    BLEND_CONST_ZERO = 0,
    BLEND_CONST_ONE,
    BLEND_CONST_SRC_COLOR,
    BLEND_CONST_ONE_MINUS_SRC_COLOR,
    BLEND_CONST_DST_COLOR,
    BLEND_CONST_ONE_MINUS_DST_COLOR,
    BLEND_CONST_SRC_ALPHA,
    BLEND_CONST_ONE_MINUS_SRC_ALPHA,
    BLEND_CONST_DST_ALPHA,
    BLEND_CONST_ONE_MINUS_DST_ALPHA,
    BLEND_CONST_SRC_ALPHA_SATURATE,
    BLEND_CONST_BLEND_FACTOR,
    BLEND_CONST_ONE_MINUS_BLEND_FACTOR,
    BLEND_CONST_COUNT,
    BLEND_CONST_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuBlendConstant;

typedef enum ECGpuCullMode
{
    CULL_MODE_NONE = 0,
    CULL_MODE_BACK,
    CULL_MODE_FRONT,
    CULL_MODE_BOTH,
    CULL_MODE_COUNT,
    CULL_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuCullMode;

typedef enum ECGpuFrontFace
{
    FRONT_FACE_CCW = 0,
    FRONT_FACE_CW,
    FRONT_FACE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuFrontFace;

typedef enum ECGpuFillMode
{
    FILL_MODE_SOLID,
    FILL_MODE_WIREFRAME,
    FILL_MODE_COUNT,
    FILL_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuFillMode;

typedef enum ECGpuVertexInputRate
{
    INPUT_RATE_VERTEX = 0,
    INPUT_RATE_INSTANCE = 1,
    INPUT_RATE_COUNT,
    INPUT_RATE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuVertexInputRate;

typedef enum ECGpuCompareMode
{
    CMP_NEVER,
    CMP_LESS,
    CMP_EQUAL,
    CMP_LEQUAL,
    CMP_GREATER,
    CMP_NOTEQUAL,
    CMP_GEQUAL,
    CMP_ALWAYS,
    CMP_COUNT,
    CMP_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuCompareMode;

typedef enum ECGpuStencilOp
{
    STENCIL_OP_KEEP,
    STENCIL_OP_SET_ZERO,
    STENCIL_OP_REPLACE,
    STENCIL_OP_INVERT,
    STENCIL_OP_INCR,
    STENCIL_OP_DECR,
    STENCIL_OP_INCR_SAT,
    STENCIL_OP_DECR_SAT,
    STENCIL_OP_COUNT,
    STENCIL_OP_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuStencilOp;

typedef enum ECGpuBlendMode
{
    BLEND_MODE_ADD,
    BLEND_MODE_SUBTRACT,
    BLEND_MODE_REVERSE_SUBTRACT,
    BLEND_MODE_MIN,
    BLEND_MODE_MAX,
    BLEND_MODE_COUNT,
    BLEND_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuBlendMode;

typedef enum ECGpuTextureDimension
{
    TEX_DIMENSION_1D,
    TEX_DIMENSION_2D,
    TEX_DIMENSION_2DMS,
    TEX_DIMENSION_3D,
    TEX_DIMENSION_CUBE,
    TEX_DIMENSION_1D_ARRAY,
    TEX_DIMENSION_2D_ARRAY,
    TEX_DIMENSION_2DMS_ARRAY,
    TEX_DIMENSION_CUBE_ARRAY,
    TEX_DIMENSION_COUNT,
    TEX_DIMENSION_UNDEFINED,
    TEX_DIMENSION_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuTextureDimension;

// Same Value As Vulkan Enumeration Bits.
typedef enum ECGpuShaderStage
{
    SHADER_STAGE_NONE = 0,

    SHADER_STAGE_VERT = 0X00000001,
    SHADER_STAGE_TESC = 0X00000002,
    SHADER_STAGE_TESE = 0X00000004,
    SHADER_STAGE_GEOM = 0X00000008,
    SHADER_STAGE_FRAG = 0X00000010,
    SHADER_STAGE_COMPUTE = 0X00000020,
    SHADER_STAGE_RAYTRACING = 0X00000040,

    SHADER_STAGE_ALL_GRAPHICS = (uint32_t)SHADER_STAGE_VERT | (uint32_t)SHADER_STAGE_TESC | (uint32_t)SHADER_STAGE_TESE | (uint32_t)SHADER_STAGE_GEOM | (uint32_t)SHADER_STAGE_FRAG,
    SHADER_STAGE_ALL_HULL = SHADER_STAGE_TESC,
    SHADER_STAGE_ALL_DOMAIN = SHADER_STAGE_TESE,
    SHADER_STAGE_COUNT = 6,
    SHADER_STAGE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuShaderStage;
typedef uint32_t CGpuShaderStages;

typedef enum ECGpuFenceStatus
{
    FENCE_STATUS_COMPLETE = 0,
    FENCE_STATUS_INCOMPLETE,
    FENCE_STATUS_NOTSUBMITTED,
    FENCE_STATUS_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuFenceStatus;

typedef enum ECGpuResourceState
{
    RESOURCE_STATE_UNDEFINED = 0,
    RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
    RESOURCE_STATE_INDEX_BUFFER = 0x2,
    RESOURCE_STATE_RENDER_TARGET = 0x4,
    RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
    RESOURCE_STATE_DEPTH_WRITE = 0x10,
    RESOURCE_STATE_DEPTH_READ = 0x20,
    RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
    RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
    RESOURCE_STATE_SHADER_RESOURCE = 0x40 | 0x80,
    RESOURCE_STATE_STREAM_OUT = 0x100,
    RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
    RESOURCE_STATE_COPY_DEST = 0x400,
    RESOURCE_STATE_COPY_SOURCE = 0x800,
    RESOURCE_STATE_GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
    RESOURCE_STATE_PRESENT = 0x1000,
    RESOURCE_STATE_COMMON = 0x2000,
    RESOURCE_STATE_ACCELERATION_STRUCTURE = 0x4000,
    RESOURCE_STATE_SHADING_RATE_SOURCE = 0x8000,
    RESOURCE_STATE_RESOLVE_DEST = 0x10000,
    RESOURCE_STATE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuResourceState;
typedef uint32_t CGpuResourceStates;

typedef enum ECGpuMemoryUsage
{
    /// No intended memory usage specified.
    MEM_USAGE_UNKNOWN = 0,
    /// Memory will be used on device only, no need to be mapped on host.
    MEM_USAGE_GPU_ONLY = 1,
    /// Memory will be mapped on host. Could be used for transfer to device.
    MEM_USAGE_CPU_ONLY = 2,
    /// Memory will be used for frequent (dynamic) updates from host and reads on device.
    MEM_USAGE_CPU_TO_GPU = 3,
    /// Memory will be used for writing on device and readback on host.
    MEM_USAGE_GPU_TO_CPU = 4,
    MEM_USAGE_COUNT,
    MEM_USAGE_MAX_ENUM = 0x7FFFFFFF
} ECGpuMemoryUsage;

typedef enum ECGpuBufferCreationFlag
{
    /// Default flag (Buffer will use aliased memory, buffer will not be cpu accessible until mapBuffer is called)
    BCF_NONE = 0x01,
    /// Buffer will allocate its own memory (COMMITTED resource)
    BCF_OWN_MEMORY_BIT = 0x02,
    /// Buffer will be persistently mapped
    BCF_PERSISTENT_MAP_BIT = 0x04,
    /// Use ESRAM to store this buffer
    BCF_ESRAM = 0x08,
    /// Flag to specify not to allocate descriptors for the resource
    BCF_NO_DESCRIPTOR_VIEW_CREATION = 0x10,
    /// Flag to specify to create GPUOnly buffer as Host visible
    BCF_HOST_VISIBLE = 0x20,
#ifdef CGPU_USE_METAL
    /* ICB Flags */
    /// Inherit pipeline in ICB
    BCF_ICB_INHERIT_PIPELINE = 0x100,
    /// Inherit pipeline in ICB
    BCF_ICB_INHERIT_BUFFERS = 0x200,
#endif
    BCF_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuBufferCreationFlag;
typedef uint32_t CGpuBufferCreationFlags;

typedef enum ECGpuTextureCreationFlag
{
    /// Default flag (Texture will use default allocation strategy decided by the api specific allocator)
    TCF_NONE = 0,
    /// Texture will allocate its own memory (COMMITTED resource)
    TCF_OWN_MEMORY_BIT = 0x01,
    /// Texture will be allocated in memory which can be shared among multiple processes
    TCF_EXPORT_BIT = 0x02,
    /// Texture will be allocated in memory which can be shared among multiple gpus
    TCF_EXPORT_ADAPTER_BIT = 0x04,
    /// Texture will be imported from a handle created in another process
    TCF_IMPORT_BIT = 0x08,
    /// Use ESRAM to store this texture
    TCF_ESRAM = 0x10,
    /// Use on-tile memory to store this texture
    TCF_ON_TILE = 0x20,
    /// Prevent compression meta data from generating (XBox)
    TCF_NO_COMPRESSION = 0x40,
    /// Force 2D instead of automatically determining dimension based on width, height, depth
    TCF_FORCE_2D = 0x80,
    /// Force 3D instead of automatically determining dimension based on width, height, depth
    TCF_FORCE_3D = 0x100,
    /// Display target
    TCF_ALLOW_DISPLAY_TARGET = 0x200,
    /// Create an sRGB texture.
    TCF_SRGB = 0x400,
    /// Create a normal map texture
    TCF_NORMAL_MAP = 0x800,
    /// Fragment mask
    TCF_FRAG_MASK = 0x2000,
    TCF_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuTextureCreationFlag;
typedef uint32_t CGpuTextureCreationFlags;

typedef enum ECGpuSampleCount
{
    SAMPLE_COUNT_1 = 1,
    SAMPLE_COUNT_2 = 2,
    SAMPLE_COUNT_4 = 4,
    SAMPLE_COUNT_8 = 8,
    SAMPLE_COUNT_16 = 16,
    SAMPLE_COUNT_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuSampleCount;

typedef enum ECGpuPipelineType
{
    PIPELINE_TYPE_NONE = 0,
    PIPELINE_TYPE_COMPUTE,
    PIPELINE_TYPE_GRAPHICS,
    PIPELINE_TYPE_RAYTRACING,
    PIPELINE_TYPE_COUNT,
    PIPELINE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuPipelineType;

typedef enum ECGpuResourceType
{
    RT_NONE = 0,
    RT_SAMPLER = 0x01,
    // SRV Read only texture
    RT_TEXTURE = (RT_SAMPLER << 1),
    /// RTV Texture
    RT_RENDER_TARGET = (RT_TEXTURE << 1),
    /// DSV Texture
    RT_DEPTH_STENCIL = (RT_RENDER_TARGET << 1),
    /// UAV Texture
    RT_RW_TEXTURE = (RT_DEPTH_STENCIL << 1),
    // SRV Read only buffer
    RT_BUFFER = (RT_RW_TEXTURE << 1),
    RT_BUFFER_RAW = (RT_BUFFER | (RT_BUFFER << 1)),
    /// UAV Buffer
    RT_RW_BUFFER = (RT_BUFFER << 2),
    RT_RW_BUFFER_RAW = (RT_RW_BUFFER | (RT_RW_BUFFER << 1)),
    /// Uniform buffer
    RT_UNIFORM_BUFFER = (RT_RW_BUFFER << 2),
    /// Push constant / Root constant
    RT_ROOT_CONSTANT = (RT_UNIFORM_BUFFER << 1),
    /// IA
    RT_VERTEX_BUFFER = (RT_ROOT_CONSTANT << 1),
    RT_INDEX_BUFFER = (RT_VERTEX_BUFFER << 1),
    RT_INDIRECT_BUFFER = (RT_INDEX_BUFFER << 1),
    /// Cubemap SRV
    RT_TEXTURE_CUBE = (RT_TEXTURE | (RT_INDIRECT_BUFFER << 1)),
    /// RTV / DSV per mip slice
    RT_RENDER_TARGET_MIP_SLICES = (RT_INDIRECT_BUFFER << 2),
    /// RTV / DSV per array slice
    RT_RENDER_TARGET_ARRAY_SLICES = (RT_RENDER_TARGET_MIP_SLICES << 1),
    /// RTV / DSV per depth slice
    RT_RENDER_TARGET_DEPTH_SLICES = (RT_RENDER_TARGET_ARRAY_SLICES << 1),
    RT_RAY_TRACING = (RT_RENDER_TARGET_DEPTH_SLICES << 1),
#if defined(CGPU_USE_VULKAN)
    /// Subpass input (descriptor type only available in Vulkan)
    RT_INPUT_ATTACHMENT = (RT_RAY_TRACING << 1),
    RT_TEXEL_BUFFER = (RT_INPUT_ATTACHMENT << 1),
    RT_RW_TEXEL_BUFFER = (RT_TEXEL_BUFFER << 1),
    RT_COMBINED_IMAGE_SAMPLER = (RT_RW_TEXEL_BUFFER << 1),
#endif
#if defined(CGPU_USE_METAL)
    RT_ARGUMENT_BUFFER = (RT_RAY_TRACING << 1),
    RT_INDIRECT_COMMAND_BUFFER = (RT_ARGUMENT_BUFFER << 1),
    RT_RENDER_PIPELINE_STATE = (RT_INDIRECT_COMMAND_BUFFER << 1),
#endif
    RT_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuResourceType;
typedef uint32_t CGpuResourceTypes;

typedef enum ECGpuTexutreViewUsage
{
    TVU_SRV = 0x01,
    TVU_RTV_DSV = 0x02,
    TVU_UAV = 0x04,
    TVU_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuTexutreViewUsage;
typedef uint32_t CGpuTexutreViewUsages;

typedef enum ECGpuTextureViewAspect
{
    TVA_COLOR = 0x01,
    TVA_DEPTH = 0x02,
    TVA_STENCIL = 0x04,
    TVA_MAX_ENUM_BIT = 0x7FFFFFFF
} ECGpuTextureViewAspect;
typedef uint32_t CGpuTextureViewAspects;

//
/* clang-format off */
static FORCEINLINE bool FormatUtil_IsDepthStencilFormat(ECGpuFormat const fmt) {
    switch(fmt) {
        case PF_D24_UNORM_S8_UINT:
        case PF_D32_SFLOAT_S8_UINT:
        case PF_D32_SFLOAT:
        case PF_X8_D24_UNORM:
        case PF_D16_UNORM:
        case PF_D16_UNORM_S8_UINT:
            return true;
        default: return false;
    }
    return false;
}

static FORCEINLINE bool FormatUtil_IsDepthOnlyFormat(ECGpuFormat const fmt) {
    switch(fmt) {
        case PF_D32_SFLOAT:
        case PF_D16_UNORM:
            return true;
        default: return false;
    }
    return false;
}

static FORCEINLINE uint32_t FormatUtil_BitSizeOfBlock(ECGpuFormat const fmt) {
	switch(fmt) {
		case PF_UNDEFINED: return 0;
		case PF_R1_UNORM: return 8;
		case PF_R2_UNORM: return 8;
		case PF_R4_UNORM: return 8;
		case PF_R4G4_UNORM: return 8;
		case PF_G4R4_UNORM: return 8;
		case PF_A8_UNORM: return 8;
		case PF_R8_UNORM: return 8;
		case PF_R8_SNORM: return 8;
		case PF_R8_UINT: return 8;
		case PF_R8_SINT: return 8;
		case PF_R8_SRGB: return 8;
		case PF_B2G3R3_UNORM: return 8;
		case PF_R4G4B4A4_UNORM: return 16;
		case PF_R4G4B4X4_UNORM: return 16;
		case PF_B4G4R4A4_UNORM: return 16;
		case PF_B4G4R4X4_UNORM: return 16;
		case PF_A4R4G4B4_UNORM: return 16;
		case PF_X4R4G4B4_UNORM: return 16;
		case PF_A4B4G4R4_UNORM: return 16;
		case PF_X4B4G4R4_UNORM: return 16;
		case PF_R5G6B5_UNORM: return 16;
		case PF_B5G6R5_UNORM: return 16;
		case PF_R5G5B5A1_UNORM: return 16;
		case PF_B5G5R5A1_UNORM: return 16;
		case PF_A1B5G5R5_UNORM: return 16;
		case PF_A1R5G5B5_UNORM: return 16;
		case PF_R5G5B5X1_UNORM: return 16;
		case PF_B5G5R5X1_UNORM: return 16;
		case PF_X1R5G5B5_UNORM: return 16;
		case PF_X1B5G5R5_UNORM: return 16;
		case PF_B2G3R3A8_UNORM: return 16;
		case PF_R8G8_UNORM: return 16;
		case PF_R8G8_SNORM: return 16;
		case PF_G8R8_UNORM: return 16;
		case PF_G8R8_SNORM: return 16;
		case PF_R8G8_UINT: return 16;
		case PF_R8G8_SINT: return 16;
		case PF_R8G8_SRGB: return 16;
		case PF_R16_UNORM: return 16;
		case PF_R16_SNORM: return 16;
		case PF_R16_UINT: return 16;
		case PF_R16_SINT: return 16;
		case PF_R16_SFLOAT: return 16;
		case PF_R16_SBFLOAT: return 16;
		case PF_R8G8B8_UNORM: return 24;
		case PF_R8G8B8_SNORM: return 24;
		case PF_R8G8B8_UINT: return 24;
		case PF_R8G8B8_SINT: return 24;
		case PF_R8G8B8_SRGB: return 24;
		case PF_B8G8R8_UNORM: return 24;
		case PF_B8G8R8_SNORM: return 24;
		case PF_B8G8R8_UINT: return 24;
		case PF_B8G8R8_SINT: return 24;
		case PF_B8G8R8_SRGB: return 24;
		case PF_R16G16B16_UNORM: return 48;
		case PF_R16G16B16_SNORM: return 48;
		case PF_R16G16B16_UINT: return 48;
		case PF_R16G16B16_SINT: return 48;
		case PF_R16G16B16_SFLOAT: return 48;
		case PF_R16G16B16_SBFLOAT: return 48;
		case PF_R16G16B16A16_UNORM: return 64;
		case PF_R16G16B16A16_SNORM: return 64;
		case PF_R16G16B16A16_UINT: return 64;
		case PF_R16G16B16A16_SINT: return 64;
		case PF_R16G16B16A16_SFLOAT: return 64;
		case PF_R16G16B16A16_SBFLOAT: return 64;
		case PF_R32G32_UINT: return 64;
		case PF_R32G32_SINT: return 64;
		case PF_R32G32_SFLOAT: return 64;
		case PF_R32G32B32_UINT: return 96;
		case PF_R32G32B32_SINT: return 96;
		case PF_R32G32B32_SFLOAT: return 96;
		case PF_R32G32B32A32_UINT: return 128;
		case PF_R32G32B32A32_SINT: return 128;
		case PF_R32G32B32A32_SFLOAT: return 128;
		case PF_R64_UINT: return 64;
		case PF_R64_SINT: return 64;
		case PF_R64_SFLOAT: return 64;
		case PF_R64G64_UINT: return 128;
		case PF_R64G64_SINT: return 128;
		case PF_R64G64_SFLOAT: return 128;
		case PF_R64G64B64_UINT: return 192;
		case PF_R64G64B64_SINT: return 192;
		case PF_R64G64B64_SFLOAT: return 192;
		case PF_R64G64B64A64_UINT: return 256;
		case PF_R64G64B64A64_SINT: return 256;
		case PF_R64G64B64A64_SFLOAT: return 256;
		case PF_D16_UNORM: return 16;
		case PF_S8_UINT: return 8;
		case PF_D32_SFLOAT_S8_UINT: return 64;
		case PF_DXBC1_RGB_UNORM: return 64;
		case PF_DXBC1_RGB_SRGB: return 64;
		case PF_DXBC1_RGBA_UNORM: return 64;
		case PF_DXBC1_RGBA_SRGB: return 64;
		case PF_DXBC2_UNORM: return 128;
		case PF_DXBC2_SRGB: return 128;
		case PF_DXBC3_UNORM: return 128;
		case PF_DXBC3_SRGB: return 128;
		case PF_DXBC4_UNORM: return 64;
		case PF_DXBC4_SNORM: return 64;
		case PF_DXBC5_UNORM: return 128;
		case PF_DXBC5_SNORM: return 128;
		case PF_DXBC6H_UFLOAT: return 128;
		case PF_DXBC6H_SFLOAT: return 128;
		case PF_DXBC7_UNORM: return 128;
		case PF_DXBC7_SRGB: return 128;
		case PF_PVRTC1_2BPP_UNORM: return 64;
		case PF_PVRTC1_4BPP_UNORM: return 64;
		case PF_PVRTC2_2BPP_UNORM: return 64;
		case PF_PVRTC2_4BPP_UNORM: return 64;
		case PF_PVRTC1_2BPP_SRGB: return 64;
		case PF_PVRTC1_4BPP_SRGB: return 64;
		case PF_PVRTC2_2BPP_SRGB: return 64;
		case PF_PVRTC2_4BPP_SRGB: return 64;
		case PF_ETC2_R8G8B8_UNORM: return 64;
		case PF_ETC2_R8G8B8_SRGB: return 64;
		case PF_ETC2_R8G8B8A1_UNORM: return 64;
		case PF_ETC2_R8G8B8A1_SRGB: return 64;
		case PF_ETC2_R8G8B8A8_UNORM: return 64;
		case PF_ETC2_R8G8B8A8_SRGB: return 64;
		case PF_ETC2_EAC_R11_UNORM: return 64;
		case PF_ETC2_EAC_R11_SNORM: return 64;
		case PF_ETC2_EAC_R11G11_UNORM: return 64;
		case PF_ETC2_EAC_R11G11_SNORM: return 64;
		case PF_ASTC_4x4_UNORM: return 128;
		case PF_ASTC_4x4_SRGB: return 128;
		case PF_ASTC_5x4_UNORM: return 128;
		case PF_ASTC_5x4_SRGB: return 128;
		case PF_ASTC_5x5_UNORM: return 128;
		case PF_ASTC_5x5_SRGB: return 128;
		case PF_ASTC_6x5_UNORM: return 128;
		case PF_ASTC_6x5_SRGB: return 128;
		case PF_ASTC_6x6_UNORM: return 128;
		case PF_ASTC_6x6_SRGB: return 128;
		case PF_ASTC_8x5_UNORM: return 128;
		case PF_ASTC_8x5_SRGB: return 128;
		case PF_ASTC_8x6_UNORM: return 128;
		case PF_ASTC_8x6_SRGB: return 128;
		case PF_ASTC_8x8_UNORM: return 128;
		case PF_ASTC_8x8_SRGB: return 128;
		case PF_ASTC_10x5_UNORM: return 128;
		case PF_ASTC_10x5_SRGB: return 128;
		case PF_ASTC_10x6_UNORM: return 128;
		case PF_ASTC_10x6_SRGB: return 128;
		case PF_ASTC_10x8_UNORM: return 128;
		case PF_ASTC_10x8_SRGB: return 128;
		case PF_ASTC_10x10_UNORM: return 128;
		case PF_ASTC_10x10_SRGB: return 128;
		case PF_ASTC_12x10_UNORM: return 128;
		case PF_ASTC_12x10_SRGB: return 128;
		case PF_ASTC_12x12_UNORM: return 128;
		case PF_ASTC_12x12_SRGB: return 128;
		case PF_CLUT_P4: return 8;
		case PF_CLUT_P4A4: return 8;
		case PF_CLUT_P8: return 8;
		case PF_CLUT_P8A8: return 16;
		case PF_G16B16G16R16_422_UNORM: return 8;
		case PF_B16G16R16G16_422_UNORM: return 8;
		case PF_R12X4G12X4B12X4A12X4_UNORM_4PACK16: return 8;
		case PF_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16: return 8;
		case PF_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16: return 8;
		case PF_R10X6G10X6B10X6A10X6_UNORM_4PACK16: return 8;
		case PF_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16: return 8;
		case PF_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16: return 8;
		case PF_G8B8G8R8_422_UNORM: return 4;
		case PF_B8G8R8G8_422_UNORM: return 4;
		default: return 32;
	}
}

static FORCEINLINE uint32_t FormatUtil_WidthOfBlock(ECGpuFormat const fmt) {
    switch(fmt) {
    case PF_UNDEFINED: return 1;
    case PF_R1_UNORM: return 8;
    case PF_R2_UNORM: return 4;
    case PF_R4_UNORM: return 2;
    case PF_DXBC1_RGB_UNORM: return 4;
    case PF_DXBC1_RGB_SRGB: return 4;
    case PF_DXBC1_RGBA_UNORM: return 4;
    case PF_DXBC1_RGBA_SRGB: return 4;
    case PF_DXBC2_UNORM: return 4;
    case PF_DXBC2_SRGB: return 4;
    case PF_DXBC3_UNORM: return 4;
    case PF_DXBC3_SRGB: return 4;
    case PF_DXBC4_UNORM: return 4;
    case PF_DXBC4_SNORM: return 4;
    case PF_DXBC5_UNORM: return 4;
    case PF_DXBC5_SNORM: return 4;
    case PF_DXBC6H_UFLOAT: return 4;
    case PF_DXBC6H_SFLOAT: return 4;
    case PF_DXBC7_UNORM: return 4;
    case PF_DXBC7_SRGB: return 4;
    case PF_PVRTC1_2BPP_UNORM: return 8;
    case PF_PVRTC1_4BPP_UNORM: return 4;
    case PF_PVRTC2_2BPP_UNORM: return 8;
    case PF_PVRTC2_4BPP_UNORM: return 4;
    case PF_PVRTC1_2BPP_SRGB: return 8;
    case PF_PVRTC1_4BPP_SRGB: return 4;
    case PF_PVRTC2_2BPP_SRGB: return 8;
    case PF_PVRTC2_4BPP_SRGB: return 4;
    case PF_ETC2_R8G8B8_UNORM: return 4;
    case PF_ETC2_R8G8B8_SRGB: return 4;
    case PF_ETC2_R8G8B8A1_UNORM: return 4;
    case PF_ETC2_R8G8B8A1_SRGB: return 4;
    case PF_ETC2_R8G8B8A8_UNORM: return 4;
    case PF_ETC2_R8G8B8A8_SRGB: return 4;
    case PF_ETC2_EAC_R11_UNORM: return 4;
    case PF_ETC2_EAC_R11_SNORM: return 4;
    case PF_ETC2_EAC_R11G11_UNORM: return 4;
    case PF_ETC2_EAC_R11G11_SNORM: return 4;
    case PF_ASTC_4x4_UNORM: return 4;
    case PF_ASTC_4x4_SRGB: return 4;
    case PF_ASTC_5x4_UNORM: return 5;
    case PF_ASTC_5x4_SRGB: return 5;
    case PF_ASTC_5x5_UNORM: return 5;
    case PF_ASTC_5x5_SRGB: return 5;
    case PF_ASTC_6x5_UNORM: return 6;
    case PF_ASTC_6x5_SRGB: return 6;
    case PF_ASTC_6x6_UNORM: return 6;
    case PF_ASTC_6x6_SRGB: return 6;
    case PF_ASTC_8x5_UNORM: return 8;
    case PF_ASTC_8x5_SRGB: return 8;
    case PF_ASTC_8x6_UNORM: return 8;
    case PF_ASTC_8x6_SRGB: return 8;
    case PF_ASTC_8x8_UNORM: return 8;
    case PF_ASTC_8x8_SRGB: return 8;
    case PF_ASTC_10x5_UNORM: return 10;
    case PF_ASTC_10x5_SRGB: return 10;
    case PF_ASTC_10x6_UNORM: return 10;
    case PF_ASTC_10x6_SRGB: return 10;
    case PF_ASTC_10x8_UNORM: return 10;
    case PF_ASTC_10x8_SRGB: return 10;
    case PF_ASTC_10x10_UNORM: return 10;
    case PF_ASTC_10x10_SRGB: return 10;
    case PF_ASTC_12x10_UNORM: return 12;
    case PF_ASTC_12x10_SRGB: return 12;
    case PF_ASTC_12x12_UNORM: return 12;
    case PF_ASTC_12x12_SRGB: return 12;
    case PF_CLUT_P4: return 2;
    default: return 1;
	}
}

static FORCEINLINE uint32_t FormatUtil_HeightOfBlock(ECGpuFormat const fmt) {
	switch(fmt) {
		case PF_UNDEFINED: return 1;
		case PF_DXBC1_RGB_UNORM: return 4;
		case PF_DXBC1_RGB_SRGB: return 4;
		case PF_DXBC1_RGBA_UNORM: return 4;
		case PF_DXBC1_RGBA_SRGB: return 4;
		case PF_DXBC2_UNORM: return 4;
		case PF_DXBC2_SRGB: return 4;
		case PF_DXBC3_UNORM: return 4;
		case PF_DXBC3_SRGB: return 4;
		case PF_DXBC4_UNORM: return 4;
		case PF_DXBC4_SNORM: return 4;
		case PF_DXBC5_UNORM: return 4;
		case PF_DXBC5_SNORM: return 4;
		case PF_DXBC6H_UFLOAT: return 4;
		case PF_DXBC6H_SFLOAT: return 4;
		case PF_DXBC7_UNORM: return 4;
		case PF_DXBC7_SRGB: return 4;
		case PF_PVRTC1_2BPP_UNORM: return 4;
		case PF_PVRTC1_4BPP_UNORM: return 4;
		case PF_PVRTC2_2BPP_UNORM: return 4;
		case PF_PVRTC2_4BPP_UNORM: return 4;
		case PF_PVRTC1_2BPP_SRGB: return 4;
		case PF_PVRTC1_4BPP_SRGB: return 4;
		case PF_PVRTC2_2BPP_SRGB: return 4;
		case PF_PVRTC2_4BPP_SRGB: return 4;
		case PF_ETC2_R8G8B8_UNORM: return 4;
		case PF_ETC2_R8G8B8_SRGB: return 4;
		case PF_ETC2_R8G8B8A1_UNORM: return 4;
		case PF_ETC2_R8G8B8A1_SRGB: return 4;
		case PF_ETC2_R8G8B8A8_UNORM: return 4;
		case PF_ETC2_R8G8B8A8_SRGB: return 4;
		case PF_ETC2_EAC_R11_UNORM: return 4;
		case PF_ETC2_EAC_R11_SNORM: return 4;
		case PF_ETC2_EAC_R11G11_UNORM: return 4;
		case PF_ETC2_EAC_R11G11_SNORM: return 4;
		case PF_ASTC_4x4_UNORM: return 4;
		case PF_ASTC_4x4_SRGB: return 4;
		case PF_ASTC_5x4_UNORM: return 4;
		case PF_ASTC_5x4_SRGB: return 4;
		case PF_ASTC_5x5_UNORM: return 5;
		case PF_ASTC_5x5_SRGB: return 5;
		case PF_ASTC_6x5_UNORM: return 5;
		case PF_ASTC_6x5_SRGB: return 5;
		case PF_ASTC_6x6_UNORM: return 6;
		case PF_ASTC_6x6_SRGB: return 6;
		case PF_ASTC_8x5_UNORM: return 5;
		case PF_ASTC_8x5_SRGB: return 5;
		case PF_ASTC_8x6_UNORM: return 6;
		case PF_ASTC_8x6_SRGB: return 6;
		case PF_ASTC_8x8_UNORM: return 8;
		case PF_ASTC_8x8_SRGB: return 8;
		case PF_ASTC_10x5_UNORM: return 5;
		case PF_ASTC_10x5_SRGB: return 5;
		case PF_ASTC_10x6_UNORM: return 6;
		case PF_ASTC_10x6_SRGB: return 6;
		case PF_ASTC_10x8_UNORM: return 8;
		case PF_ASTC_10x8_SRGB: return 8;
		case PF_ASTC_10x10_UNORM: return 10;
		case PF_ASTC_10x10_SRGB: return 10;
		case PF_ASTC_12x10_UNORM: return 10;
		case PF_ASTC_12x10_SRGB: return 10;
		case PF_ASTC_12x12_UNORM: return 12;
		case PF_ASTC_12x12_SRGB: return 12;
		default: return 1;
	}
}
/* clang-format on */

#ifdef __cplusplus
} // end extern "C"
#endif
