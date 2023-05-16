pub const __builtin_bswap16 = @import("std").zig.c_builtins.__builtin_bswap16;
pub const __builtin_bswap32 = @import("std").zig.c_builtins.__builtin_bswap32;
pub const __builtin_bswap64 = @import("std").zig.c_builtins.__builtin_bswap64;
pub const __builtin_signbit = @import("std").zig.c_builtins.__builtin_signbit;
pub const __builtin_signbitf = @import("std").zig.c_builtins.__builtin_signbitf;
pub const __builtin_popcount = @import("std").zig.c_builtins.__builtin_popcount;
pub const __builtin_ctz = @import("std").zig.c_builtins.__builtin_ctz;
pub const __builtin_clz = @import("std").zig.c_builtins.__builtin_clz;
pub const __builtin_sqrt = @import("std").zig.c_builtins.__builtin_sqrt;
pub const __builtin_sqrtf = @import("std").zig.c_builtins.__builtin_sqrtf;
pub const __builtin_sin = @import("std").zig.c_builtins.__builtin_sin;
pub const __builtin_sinf = @import("std").zig.c_builtins.__builtin_sinf;
pub const __builtin_cos = @import("std").zig.c_builtins.__builtin_cos;
pub const __builtin_cosf = @import("std").zig.c_builtins.__builtin_cosf;
pub const __builtin_exp = @import("std").zig.c_builtins.__builtin_exp;
pub const __builtin_expf = @import("std").zig.c_builtins.__builtin_expf;
pub const __builtin_exp2 = @import("std").zig.c_builtins.__builtin_exp2;
pub const __builtin_exp2f = @import("std").zig.c_builtins.__builtin_exp2f;
pub const __builtin_log = @import("std").zig.c_builtins.__builtin_log;
pub const __builtin_logf = @import("std").zig.c_builtins.__builtin_logf;
pub const __builtin_log2 = @import("std").zig.c_builtins.__builtin_log2;
pub const __builtin_log2f = @import("std").zig.c_builtins.__builtin_log2f;
pub const __builtin_log10 = @import("std").zig.c_builtins.__builtin_log10;
pub const __builtin_log10f = @import("std").zig.c_builtins.__builtin_log10f;
pub const __builtin_abs = @import("std").zig.c_builtins.__builtin_abs;
pub const __builtin_fabs = @import("std").zig.c_builtins.__builtin_fabs;
pub const __builtin_fabsf = @import("std").zig.c_builtins.__builtin_fabsf;
pub const __builtin_floor = @import("std").zig.c_builtins.__builtin_floor;
pub const __builtin_floorf = @import("std").zig.c_builtins.__builtin_floorf;
pub const __builtin_ceil = @import("std").zig.c_builtins.__builtin_ceil;
pub const __builtin_ceilf = @import("std").zig.c_builtins.__builtin_ceilf;
pub const __builtin_trunc = @import("std").zig.c_builtins.__builtin_trunc;
pub const __builtin_truncf = @import("std").zig.c_builtins.__builtin_truncf;
pub const __builtin_round = @import("std").zig.c_builtins.__builtin_round;
pub const __builtin_roundf = @import("std").zig.c_builtins.__builtin_roundf;
pub const __builtin_strlen = @import("std").zig.c_builtins.__builtin_strlen;
pub const __builtin_strcmp = @import("std").zig.c_builtins.__builtin_strcmp;
pub const __builtin_object_size = @import("std").zig.c_builtins.__builtin_object_size;
pub const __builtin___memset_chk = @import("std").zig.c_builtins.__builtin___memset_chk;
pub const __builtin_memset = @import("std").zig.c_builtins.__builtin_memset;
pub const __builtin___memcpy_chk = @import("std").zig.c_builtins.__builtin___memcpy_chk;
pub const __builtin_memcpy = @import("std").zig.c_builtins.__builtin_memcpy;
pub const __builtin_expect = @import("std").zig.c_builtins.__builtin_expect;
pub const __builtin_nanf = @import("std").zig.c_builtins.__builtin_nanf;
pub const __builtin_huge_valf = @import("std").zig.c_builtins.__builtin_huge_valf;
pub const __builtin_inff = @import("std").zig.c_builtins.__builtin_inff;
pub const __builtin_isnan = @import("std").zig.c_builtins.__builtin_isnan;
pub const __builtin_isinf = @import("std").zig.c_builtins.__builtin_isinf;
pub const __builtin_isinf_sign = @import("std").zig.c_builtins.__builtin_isinf_sign;
pub const __has_builtin = @import("std").zig.c_builtins.__has_builtin;
pub const __builtin_assume = @import("std").zig.c_builtins.__builtin_assume;
pub const __builtin_unreachable = @import("std").zig.c_builtins.__builtin_unreachable;
pub const __builtin_constant_p = @import("std").zig.c_builtins.__builtin_constant_p;
pub const __builtin_mul_overflow = @import("std").zig.c_builtins.__builtin_mul_overflow;
pub const int_least64_t = i64;
pub const uint_least64_t = u64;
pub const int_fast64_t = i64;
pub const uint_fast64_t = u64;
pub const int_least32_t = i32;
pub const uint_least32_t = u32;
pub const int_fast32_t = i32;
pub const uint_fast32_t = u32;
pub const int_least16_t = i16;
pub const uint_least16_t = u16;
pub const int_fast16_t = i16;
pub const uint_fast16_t = u16;
pub const int_least8_t = i8;
pub const uint_least8_t = u8;
pub const int_fast8_t = i8;
pub const uint_fast8_t = u8;
pub const intmax_t = c_longlong;
pub const uintmax_t = c_ulonglong;
pub const @"struct_$T" = extern struct {
    @"_": u32,
};
pub const @"$T" = @"struct_$T";
pub const @"struct_$Super" = extern struct {
    @"_": u32,
};
pub const @"$Super" = @"struct_$Super";
pub const @"struct_$Owner" = extern struct {
    @"_": u32,
};
pub const @"$Owner" = @"struct_$Owner";
pub const @"struct_$Module" = extern struct {
    @"_": u32,
};
pub const @"$Module" = @"struct_$Module";
pub extern var @"$name": [*c]const u8;
pub const char8_t = u8;
pub const CGPU_NVAPI_OK: c_int = 0;
pub const CGPU_NVAPI_NONE: c_int = 1;
pub const CGPU_NVAPI_ERROR: c_int = -1;
pub const CGPU_NVAPI_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUNvAPI_Status = c_int;
pub const ECGPUNvAPI_Status = enum_ECGPUNvAPI_Status;
pub const CGPU_AGS_SUCCESS: c_int = 0;
pub const CGPU_AGS_FAILURE: c_int = 1;
pub const CGPU_AGS_INVALID_ARGS: c_int = 2;
pub const CGPU_AGS_OUT_OF_MEMORY: c_int = 3;
pub const CGPU_AGS_MISSING_D3D_DLL: c_int = 4;
pub const CGPU_AGS_LEGACY_DRIVER: c_int = 5;
pub const CGPU_AGS_NO_AMD_DRIVER_INSTALLED: c_int = 6;
pub const CGPU_AGS_EXTENSION_NOT_SUPPORTED: c_int = 7;
pub const CGPU_AGS_ADL_FAILURE: c_int = 8;
pub const CGPU_AGS_DX_FAILURE: c_int = 9;
pub const CGPU_AGS_NONE: c_int = 10;
pub const CGPU_AGS_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUAGSReturnCode = c_uint;
pub const ECGPUAGSReturnCode = enum_ECGPUAGSReturnCode;
pub const CGPU_FORMAT_UNDEFINED: c_int = 0;
pub const CGPU_FORMAT_R1_UNORM: c_int = 1;
pub const CGPU_FORMAT_R2_UNORM: c_int = 2;
pub const CGPU_FORMAT_R4_UNORM: c_int = 3;
pub const CGPU_FORMAT_R4G4_UNORM: c_int = 4;
pub const CGPU_FORMAT_G4R4_UNORM: c_int = 5;
pub const CGPU_FORMAT_A8_UNORM: c_int = 6;
pub const CGPU_FORMAT_R8_UNORM: c_int = 7;
pub const CGPU_FORMAT_R8_SNORM: c_int = 8;
pub const CGPU_FORMAT_R8_UINT: c_int = 9;
pub const CGPU_FORMAT_R8_SINT: c_int = 10;
pub const CGPU_FORMAT_R8_SRGB: c_int = 11;
pub const CGPU_FORMAT_B2G3R3_UNORM: c_int = 12;
pub const CGPU_FORMAT_R4G4B4A4_UNORM: c_int = 13;
pub const CGPU_FORMAT_R4G4B4X4_UNORM: c_int = 14;
pub const CGPU_FORMAT_B4G4R4A4_UNORM: c_int = 15;
pub const CGPU_FORMAT_B4G4R4X4_UNORM: c_int = 16;
pub const CGPU_FORMAT_A4R4G4B4_UNORM: c_int = 17;
pub const CGPU_FORMAT_X4R4G4B4_UNORM: c_int = 18;
pub const CGPU_FORMAT_A4B4G4R4_UNORM: c_int = 19;
pub const CGPU_FORMAT_X4B4G4R4_UNORM: c_int = 20;
pub const CGPU_FORMAT_R5G6B5_UNORM: c_int = 21;
pub const CGPU_FORMAT_B5G6R5_UNORM: c_int = 22;
pub const CGPU_FORMAT_R5G5B5A1_UNORM: c_int = 23;
pub const CGPU_FORMAT_B5G5R5A1_UNORM: c_int = 24;
pub const CGPU_FORMAT_A1B5G5R5_UNORM: c_int = 25;
pub const CGPU_FORMAT_A1R5G5B5_UNORM: c_int = 26;
pub const CGPU_FORMAT_R5G5B5X1_UNORM: c_int = 27;
pub const CGPU_FORMAT_B5G5R5X1_UNORM: c_int = 28;
pub const CGPU_FORMAT_X1R5G5B5_UNORM: c_int = 29;
pub const CGPU_FORMAT_X1B5G5R5_UNORM: c_int = 30;
pub const CGPU_FORMAT_B2G3R3A8_UNORM: c_int = 31;
pub const CGPU_FORMAT_R8G8_UNORM: c_int = 32;
pub const CGPU_FORMAT_R8G8_SNORM: c_int = 33;
pub const CGPU_FORMAT_G8R8_UNORM: c_int = 34;
pub const CGPU_FORMAT_G8R8_SNORM: c_int = 35;
pub const CGPU_FORMAT_R8G8_UINT: c_int = 36;
pub const CGPU_FORMAT_R8G8_SINT: c_int = 37;
pub const CGPU_FORMAT_R8G8_SRGB: c_int = 38;
pub const CGPU_FORMAT_R16_UNORM: c_int = 39;
pub const CGPU_FORMAT_R16_SNORM: c_int = 40;
pub const CGPU_FORMAT_R16_UINT: c_int = 41;
pub const CGPU_FORMAT_R16_SINT: c_int = 42;
pub const CGPU_FORMAT_R16_SFLOAT: c_int = 43;
pub const CGPU_FORMAT_R16_SBFLOAT: c_int = 44;
pub const CGPU_FORMAT_R8G8B8_UNORM: c_int = 45;
pub const CGPU_FORMAT_R8G8B8_SNORM: c_int = 46;
pub const CGPU_FORMAT_R8G8B8_UINT: c_int = 47;
pub const CGPU_FORMAT_R8G8B8_SINT: c_int = 48;
pub const CGPU_FORMAT_R8G8B8_SRGB: c_int = 49;
pub const CGPU_FORMAT_B8G8R8_UNORM: c_int = 50;
pub const CGPU_FORMAT_B8G8R8_SNORM: c_int = 51;
pub const CGPU_FORMAT_B8G8R8_UINT: c_int = 52;
pub const CGPU_FORMAT_B8G8R8_SINT: c_int = 53;
pub const CGPU_FORMAT_B8G8R8_SRGB: c_int = 54;
pub const CGPU_FORMAT_R8G8B8A8_UNORM: c_int = 55;
pub const CGPU_FORMAT_R8G8B8A8_SNORM: c_int = 56;
pub const CGPU_FORMAT_R8G8B8A8_UINT: c_int = 57;
pub const CGPU_FORMAT_R8G8B8A8_SINT: c_int = 58;
pub const CGPU_FORMAT_R8G8B8A8_SRGB: c_int = 59;
pub const CGPU_FORMAT_B8G8R8A8_UNORM: c_int = 60;
pub const CGPU_FORMAT_B8G8R8A8_SNORM: c_int = 61;
pub const CGPU_FORMAT_B8G8R8A8_UINT: c_int = 62;
pub const CGPU_FORMAT_B8G8R8A8_SINT: c_int = 63;
pub const CGPU_FORMAT_B8G8R8A8_SRGB: c_int = 64;
pub const CGPU_FORMAT_R8G8B8X8_UNORM: c_int = 65;
pub const CGPU_FORMAT_B8G8R8X8_UNORM: c_int = 66;
pub const CGPU_FORMAT_R16G16_UNORM: c_int = 67;
pub const CGPU_FORMAT_G16R16_UNORM: c_int = 68;
pub const CGPU_FORMAT_R16G16_SNORM: c_int = 69;
pub const CGPU_FORMAT_G16R16_SNORM: c_int = 70;
pub const CGPU_FORMAT_R16G16_UINT: c_int = 71;
pub const CGPU_FORMAT_R16G16_SINT: c_int = 72;
pub const CGPU_FORMAT_R16G16_SFLOAT: c_int = 73;
pub const CGPU_FORMAT_R16G16_SBFLOAT: c_int = 74;
pub const CGPU_FORMAT_R32_UINT: c_int = 75;
pub const CGPU_FORMAT_R32_SINT: c_int = 76;
pub const CGPU_FORMAT_R32_SFLOAT: c_int = 77;
pub const CGPU_FORMAT_A2R10G10B10_UNORM: c_int = 78;
pub const CGPU_FORMAT_A2R10G10B10_UINT: c_int = 79;
pub const CGPU_FORMAT_A2R10G10B10_SNORM: c_int = 80;
pub const CGPU_FORMAT_A2R10G10B10_SINT: c_int = 81;
pub const CGPU_FORMAT_A2B10G10R10_UNORM: c_int = 82;
pub const CGPU_FORMAT_A2B10G10R10_UINT: c_int = 83;
pub const CGPU_FORMAT_A2B10G10R10_SNORM: c_int = 84;
pub const CGPU_FORMAT_A2B10G10R10_SINT: c_int = 85;
pub const CGPU_FORMAT_R10G10B10A2_UNORM: c_int = 86;
pub const CGPU_FORMAT_R10G10B10A2_UINT: c_int = 87;
pub const CGPU_FORMAT_R10G10B10A2_SNORM: c_int = 88;
pub const CGPU_FORMAT_R10G10B10A2_SINT: c_int = 89;
pub const CGPU_FORMAT_B10G10R10A2_UNORM: c_int = 90;
pub const CGPU_FORMAT_B10G10R10A2_UINT: c_int = 91;
pub const CGPU_FORMAT_B10G10R10A2_SNORM: c_int = 92;
pub const CGPU_FORMAT_B10G10R10A2_SINT: c_int = 93;
pub const CGPU_FORMAT_B10G11R11_UFLOAT: c_int = 94;
pub const CGPU_FORMAT_E5B9G9R9_UFLOAT: c_int = 95;
pub const CGPU_FORMAT_R16G16B16_UNORM: c_int = 96;
pub const CGPU_FORMAT_R16G16B16_SNORM: c_int = 97;
pub const CGPU_FORMAT_R16G16B16_UINT: c_int = 98;
pub const CGPU_FORMAT_R16G16B16_SINT: c_int = 99;
pub const CGPU_FORMAT_R16G16B16_SFLOAT: c_int = 100;
pub const CGPU_FORMAT_R16G16B16_SBFLOAT: c_int = 101;
pub const CGPU_FORMAT_R16G16B16A16_UNORM: c_int = 102;
pub const CGPU_FORMAT_R16G16B16A16_SNORM: c_int = 103;
pub const CGPU_FORMAT_R16G16B16A16_UINT: c_int = 104;
pub const CGPU_FORMAT_R16G16B16A16_SINT: c_int = 105;
pub const CGPU_FORMAT_R16G16B16A16_SFLOAT: c_int = 106;
pub const CGPU_FORMAT_R16G16B16A16_SBFLOAT: c_int = 107;
pub const CGPU_FORMAT_R32G32_UINT: c_int = 108;
pub const CGPU_FORMAT_R32G32_SINT: c_int = 109;
pub const CGPU_FORMAT_R32G32_SFLOAT: c_int = 110;
pub const CGPU_FORMAT_R32G32B32_UINT: c_int = 111;
pub const CGPU_FORMAT_R32G32B32_SINT: c_int = 112;
pub const CGPU_FORMAT_R32G32B32_SFLOAT: c_int = 113;
pub const CGPU_FORMAT_R32G32B32A32_UINT: c_int = 114;
pub const CGPU_FORMAT_R32G32B32A32_SINT: c_int = 115;
pub const CGPU_FORMAT_R32G32B32A32_SFLOAT: c_int = 116;
pub const CGPU_FORMAT_R64_UINT: c_int = 117;
pub const CGPU_FORMAT_R64_SINT: c_int = 118;
pub const CGPU_FORMAT_R64_SFLOAT: c_int = 119;
pub const CGPU_FORMAT_R64G64_UINT: c_int = 120;
pub const CGPU_FORMAT_R64G64_SINT: c_int = 121;
pub const CGPU_FORMAT_R64G64_SFLOAT: c_int = 122;
pub const CGPU_FORMAT_R64G64B64_UINT: c_int = 123;
pub const CGPU_FORMAT_R64G64B64_SINT: c_int = 124;
pub const CGPU_FORMAT_R64G64B64_SFLOAT: c_int = 125;
pub const CGPU_FORMAT_R64G64B64A64_UINT: c_int = 126;
pub const CGPU_FORMAT_R64G64B64A64_SINT: c_int = 127;
pub const CGPU_FORMAT_R64G64B64A64_SFLOAT: c_int = 128;
pub const CGPU_FORMAT_D16_UNORM: c_int = 129;
pub const CGPU_FORMAT_X8_D24_UNORM: c_int = 130;
pub const CGPU_FORMAT_D32_SFLOAT: c_int = 131;
pub const CGPU_FORMAT_S8_UINT: c_int = 132;
pub const CGPU_FORMAT_D16_UNORM_S8_UINT: c_int = 133;
pub const CGPU_FORMAT_D24_UNORM_S8_UINT: c_int = 134;
pub const CGPU_FORMAT_D32_SFLOAT_S8_UINT: c_int = 135;
pub const CGPU_FORMAT_DXBC1_RGB_UNORM: c_int = 136;
pub const CGPU_FORMAT_DXBC1_RGB_SRGB: c_int = 137;
pub const CGPU_FORMAT_DXBC1_RGBA_UNORM: c_int = 138;
pub const CGPU_FORMAT_DXBC1_RGBA_SRGB: c_int = 139;
pub const CGPU_FORMAT_DXBC2_UNORM: c_int = 140;
pub const CGPU_FORMAT_DXBC2_SRGB: c_int = 141;
pub const CGPU_FORMAT_DXBC3_UNORM: c_int = 142;
pub const CGPU_FORMAT_DXBC3_SRGB: c_int = 143;
pub const CGPU_FORMAT_DXBC4_UNORM: c_int = 144;
pub const CGPU_FORMAT_DXBC4_SNORM: c_int = 145;
pub const CGPU_FORMAT_DXBC5_UNORM: c_int = 146;
pub const CGPU_FORMAT_DXBC5_SNORM: c_int = 147;
pub const CGPU_FORMAT_DXBC6H_UFLOAT: c_int = 148;
pub const CGPU_FORMAT_DXBC6H_SFLOAT: c_int = 149;
pub const CGPU_FORMAT_DXBC7_UNORM: c_int = 150;
pub const CGPU_FORMAT_DXBC7_SRGB: c_int = 151;
pub const CGPU_FORMAT_PVRTC1_2BPP_UNORM: c_int = 152;
pub const CGPU_FORMAT_PVRTC1_4BPP_UNORM: c_int = 153;
pub const CGPU_FORMAT_PVRTC2_2BPP_UNORM: c_int = 154;
pub const CGPU_FORMAT_PVRTC2_4BPP_UNORM: c_int = 155;
pub const CGPU_FORMAT_PVRTC1_2BPP_SRGB: c_int = 156;
pub const CGPU_FORMAT_PVRTC1_4BPP_SRGB: c_int = 157;
pub const CGPU_FORMAT_PVRTC2_2BPP_SRGB: c_int = 158;
pub const CGPU_FORMAT_PVRTC2_4BPP_SRGB: c_int = 159;
pub const CGPU_FORMAT_ETC2_R8G8B8_UNORM: c_int = 160;
pub const CGPU_FORMAT_ETC2_R8G8B8_SRGB: c_int = 161;
pub const CGPU_FORMAT_ETC2_R8G8B8A1_UNORM: c_int = 162;
pub const CGPU_FORMAT_ETC2_R8G8B8A1_SRGB: c_int = 163;
pub const CGPU_FORMAT_ETC2_R8G8B8A8_UNORM: c_int = 164;
pub const CGPU_FORMAT_ETC2_R8G8B8A8_SRGB: c_int = 165;
pub const CGPU_FORMAT_ETC2_EAC_R11_UNORM: c_int = 166;
pub const CGPU_FORMAT_ETC2_EAC_R11_SNORM: c_int = 167;
pub const CGPU_FORMAT_ETC2_EAC_R11G11_UNORM: c_int = 168;
pub const CGPU_FORMAT_ETC2_EAC_R11G11_SNORM: c_int = 169;
pub const CGPU_FORMAT_ASTC_4x4_UNORM: c_int = 170;
pub const CGPU_FORMAT_ASTC_4x4_SRGB: c_int = 171;
pub const CGPU_FORMAT_ASTC_5x4_UNORM: c_int = 172;
pub const CGPU_FORMAT_ASTC_5x4_SRGB: c_int = 173;
pub const CGPU_FORMAT_ASTC_5x5_UNORM: c_int = 174;
pub const CGPU_FORMAT_ASTC_5x5_SRGB: c_int = 175;
pub const CGPU_FORMAT_ASTC_6x5_UNORM: c_int = 176;
pub const CGPU_FORMAT_ASTC_6x5_SRGB: c_int = 177;
pub const CGPU_FORMAT_ASTC_6x6_UNORM: c_int = 178;
pub const CGPU_FORMAT_ASTC_6x6_SRGB: c_int = 179;
pub const CGPU_FORMAT_ASTC_8x5_UNORM: c_int = 180;
pub const CGPU_FORMAT_ASTC_8x5_SRGB: c_int = 181;
pub const CGPU_FORMAT_ASTC_8x6_UNORM: c_int = 182;
pub const CGPU_FORMAT_ASTC_8x6_SRGB: c_int = 183;
pub const CGPU_FORMAT_ASTC_8x8_UNORM: c_int = 184;
pub const CGPU_FORMAT_ASTC_8x8_SRGB: c_int = 185;
pub const CGPU_FORMAT_ASTC_10x5_UNORM: c_int = 186;
pub const CGPU_FORMAT_ASTC_10x5_SRGB: c_int = 187;
pub const CGPU_FORMAT_ASTC_10x6_UNORM: c_int = 188;
pub const CGPU_FORMAT_ASTC_10x6_SRGB: c_int = 189;
pub const CGPU_FORMAT_ASTC_10x8_UNORM: c_int = 190;
pub const CGPU_FORMAT_ASTC_10x8_SRGB: c_int = 191;
pub const CGPU_FORMAT_ASTC_10x10_UNORM: c_int = 192;
pub const CGPU_FORMAT_ASTC_10x10_SRGB: c_int = 193;
pub const CGPU_FORMAT_ASTC_12x10_UNORM: c_int = 194;
pub const CGPU_FORMAT_ASTC_12x10_SRGB: c_int = 195;
pub const CGPU_FORMAT_ASTC_12x12_UNORM: c_int = 196;
pub const CGPU_FORMAT_ASTC_12x12_SRGB: c_int = 197;
pub const CGPU_FORMAT_CLUT_P4: c_int = 198;
pub const CGPU_FORMAT_CLUT_P4A4: c_int = 199;
pub const CGPU_FORMAT_CLUT_P8: c_int = 200;
pub const CGPU_FORMAT_CLUT_P8A8: c_int = 201;
pub const CGPU_FORMAT_R4G4B4A4_UNORM_PACK16: c_int = 202;
pub const CGPU_FORMAT_B4G4R4A4_UNORM_PACK16: c_int = 203;
pub const CGPU_FORMAT_R5G6B5_UNORM_PACK16: c_int = 204;
pub const CGPU_FORMAT_B5G6R5_UNORM_PACK16: c_int = 205;
pub const CGPU_FORMAT_R5G5B5A1_UNORM_PACK16: c_int = 206;
pub const CGPU_FORMAT_B5G5R5A1_UNORM_PACK16: c_int = 207;
pub const CGPU_FORMAT_A1R5G5B5_UNORM_PACK16: c_int = 208;
pub const CGPU_FORMAT_G16B16G16R16_422_UNORM: c_int = 209;
pub const CGPU_FORMAT_B16G16R16G16_422_UNORM: c_int = 210;
pub const CGPU_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16: c_int = 211;
pub const CGPU_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16: c_int = 212;
pub const CGPU_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16: c_int = 213;
pub const CGPU_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16: c_int = 214;
pub const CGPU_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16: c_int = 215;
pub const CGPU_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16: c_int = 216;
pub const CGPU_FORMAT_G8B8G8R8_422_UNORM: c_int = 217;
pub const CGPU_FORMAT_B8G8R8G8_422_UNORM: c_int = 218;
pub const CGPU_FORMAT_G8_B8_R8_3PLANE_420_UNORM: c_int = 219;
pub const CGPU_FORMAT_G8_B8R8_2PLANE_420_UNORM: c_int = 220;
pub const CGPU_FORMAT_G8_B8_R8_3PLANE_422_UNORM: c_int = 221;
pub const CGPU_FORMAT_G8_B8R8_2PLANE_422_UNORM: c_int = 222;
pub const CGPU_FORMAT_G8_B8_R8_3PLANE_444_UNORM: c_int = 223;
pub const CGPU_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16: c_int = 224;
pub const CGPU_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16: c_int = 225;
pub const CGPU_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16: c_int = 226;
pub const CGPU_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16: c_int = 227;
pub const CGPU_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16: c_int = 228;
pub const CGPU_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16: c_int = 229;
pub const CGPU_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16: c_int = 230;
pub const CGPU_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16: c_int = 231;
pub const CGPU_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16: c_int = 232;
pub const CGPU_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16: c_int = 233;
pub const CGPU_FORMAT_G16_B16_R16_3PLANE_420_UNORM: c_int = 234;
pub const CGPU_FORMAT_G16_B16_R16_3PLANE_422_UNORM: c_int = 235;
pub const CGPU_FORMAT_G16_B16_R16_3PLANE_444_UNORM: c_int = 236;
pub const CGPU_FORMAT_G16_B16R16_2PLANE_420_UNORM: c_int = 237;
pub const CGPU_FORMAT_G16_B16R16_2PLANE_422_UNORM: c_int = 238;
pub const CGPU_FORMAT_COUNT: c_int = 239;
pub const CGPU_FORMAT_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUFormat = c_uint;
pub const ECGPUFormat = enum_ECGPUFormat;
pub const CGPU_CHANNEL_INVALID: c_int = 0;
pub const CGPU_CHANNEL_R: c_int = 1;
pub const CGPU_CHANNEL_G: c_int = 2;
pub const CGPU_CHANNEL_B: c_int = 4;
pub const CGPU_CHANNEL_A: c_int = 8;
pub const CGPU_CHANNEL_RG: c_int = 3;
pub const CGPU_CHANNEL_RGB: c_int = 7;
pub const CGPU_CHANNEL_RGBA: c_int = 15;
pub const CGPU_CHANNEL_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUChannelBit = c_uint;
pub const ECGPUChannelBit = enum_ECGPUChannelBit;
pub const CGPU_SLOT_0: c_int = 1;
pub const CGPU_SLOT_1: c_int = 2;
pub const CGPU_SLOT_2: c_int = 4;
pub const CGPU_SLOT_3: c_int = 8;
pub const CGPU_SLOT_4: c_int = 16;
pub const CGPU_SLOT_5: c_int = 32;
pub const CGPU_SLOT_6: c_int = 64;
pub const CGPU_SLOT_7: c_int = 128;
pub const enum_ECGPUSlotMaskBit = c_uint;
pub const ECGPUSlotMaskBit = enum_ECGPUSlotMaskBit;
pub const ECGPUSlotMask = u32;
pub const CGPU_FILTER_TYPE_NEAREST: c_int = 0;
pub const CGPU_FILTER_TYPE_LINEAR: c_int = 1;
pub const CGPU_FILTER_TYPE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUFilterType = c_uint;
pub const ECGPUFilterType = enum_ECGPUFilterType;
pub const CGPU_ADDRESS_MODE_MIRROR: c_int = 0;
pub const CGPU_ADDRESS_MODE_REPEAT: c_int = 1;
pub const CGPU_ADDRESS_MODE_CLAMP_TO_EDGE: c_int = 2;
pub const CGPU_ADDRESS_MODE_CLAMP_TO_BORDER: c_int = 3;
pub const CGPU_ADDRESS_MODE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUAddressMode = c_uint;
pub const ECGPUAddressMode = enum_ECGPUAddressMode;
pub const CGPU_MIPMAP_MODE_NEAREST: c_int = 0;
pub const CGPU_MIPMAP_MODE_LINEAR: c_int = 1;
pub const CGPU_MIPMAP_MODE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUMipMapMode = c_uint;
pub const ECGPUMipMapMode = enum_ECGPUMipMapMode;
pub const CGPU_LOAD_ACTION_DONTCARE: c_int = 0;
pub const CGPU_LOAD_ACTION_LOAD: c_int = 1;
pub const CGPU_LOAD_ACTION_CLEAR: c_int = 2;
pub const CGPU_LOAD_ACTION_COUNT: c_int = 3;
pub const CGPU_LOAD_ACTION_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPULoadAction = c_uint;
pub const ECGPULoadAction = enum_ECGPULoadAction;
pub const CGPU_STORE_ACTION_STORE: c_int = 0;
pub const CGPU_STORE_ACTION_DISCARD: c_int = 1;
pub const CGPU_STORE_ACTION_COUNT: c_int = 2;
pub const CGPU_STORE_ACTION_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUStoreAction = c_uint;
pub const ECGPUStoreAction = enum_ECGPUStoreAction;
pub const CGPU_PRIM_TOPO_POINT_LIST: c_int = 0;
pub const CGPU_PRIM_TOPO_LINE_LIST: c_int = 1;
pub const CGPU_PRIM_TOPO_LINE_STRIP: c_int = 2;
pub const CGPU_PRIM_TOPO_TRI_LIST: c_int = 3;
pub const CGPU_PRIM_TOPO_TRI_STRIP: c_int = 4;
pub const CGPU_PRIM_TOPO_PATCH_LIST: c_int = 5;
pub const CGPU_PRIM_TOPO_COUNT: c_int = 6;
pub const CGPU_PRIM_TOPO_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUPrimitiveTopology = c_uint;
pub const ECGPUPrimitiveTopology = enum_ECGPUPrimitiveTopology;
pub const CGPU_BLEND_CONST_ZERO: c_int = 0;
pub const CGPU_BLEND_CONST_ONE: c_int = 1;
pub const CGPU_BLEND_CONST_SRC_COLOR: c_int = 2;
pub const CGPU_BLEND_CONST_ONE_MINUS_SRC_COLOR: c_int = 3;
pub const CGPU_BLEND_CONST_DST_COLOR: c_int = 4;
pub const CGPU_BLEND_CONST_ONE_MINUS_DST_COLOR: c_int = 5;
pub const CGPU_BLEND_CONST_SRC_ALPHA: c_int = 6;
pub const CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA: c_int = 7;
pub const CGPU_BLEND_CONST_DST_ALPHA: c_int = 8;
pub const CGPU_BLEND_CONST_ONE_MINUS_DST_ALPHA: c_int = 9;
pub const CGPU_BLEND_CONST_SRC_ALPHA_SATURATE: c_int = 10;
pub const CGPU_BLEND_CONST_BLEND_FACTOR: c_int = 11;
pub const CGPU_BLEND_CONST_ONE_MINUS_BLEND_FACTOR: c_int = 12;
pub const CGPU_BLEND_CONST_COUNT: c_int = 13;
pub const CGPU_BLEND_CONST_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUBlendConstant = c_uint;
pub const ECGPUBlendConstant = enum_ECGPUBlendConstant;
pub const CGPU_CULL_MODE_NONE: c_int = 0;
pub const CGPU_CULL_MODE_BACK: c_int = 1;
pub const CGPU_CULL_MODE_FRONT: c_int = 2;
pub const CGPU_CULL_MODE_BOTH: c_int = 3;
pub const CGPU_CULL_MODE_COUNT: c_int = 4;
pub const CGPU_CULL_MODE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUCullMode = c_uint;
pub const ECGPUCullMode = enum_ECGPUCullMode;
pub const CGPU_FRONT_FACE_CCW: c_int = 0;
pub const CGPU_FRONT_FACE_CW: c_int = 1;
pub const CGPU_FRONT_FACE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUFrontFace = c_uint;
pub const ECGPUFrontFace = enum_ECGPUFrontFace;
pub const CGPU_FILL_MODE_SOLID: c_int = 0;
pub const CGPU_FILL_MODE_WIREFRAME: c_int = 1;
pub const CGPU_FILL_MODE_COUNT: c_int = 2;
pub const CGPU_FILL_MODE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUFillMode = c_uint;
pub const ECGPUFillMode = enum_ECGPUFillMode;
pub const CGPU_INPUT_RATE_VERTEX: c_int = 0;
pub const CGPU_INPUT_RATE_INSTANCE: c_int = 1;
pub const CGPU_INPUT_RATE_COUNT: c_int = 2;
pub const CGPU_INPUT_RATE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUVertexInputRate = c_uint;
pub const ECGPUVertexInputRate = enum_ECGPUVertexInputRate;
pub const CGPU_CMP_NEVER: c_int = 0;
pub const CGPU_CMP_LESS: c_int = 1;
pub const CGPU_CMP_EQUAL: c_int = 2;
pub const CGPU_CMP_LEQUAL: c_int = 3;
pub const CGPU_CMP_GREATER: c_int = 4;
pub const CGPU_CMP_NOTEQUAL: c_int = 5;
pub const CGPU_CMP_GEQUAL: c_int = 6;
pub const CGPU_CMP_ALWAYS: c_int = 7;
pub const CGPU_CMP_COUNT: c_int = 8;
pub const CGPU_CMP_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUCompareMode = c_uint;
pub const ECGPUCompareMode = enum_ECGPUCompareMode;
pub const CGPU_STENCIL_OP_KEEP: c_int = 0;
pub const CGPU_STENCIL_OP_SET_ZERO: c_int = 1;
pub const CGPU_STENCIL_OP_REPLACE: c_int = 2;
pub const CGPU_STENCIL_OP_INVERT: c_int = 3;
pub const CGPU_STENCIL_OP_INCR: c_int = 4;
pub const CGPU_STENCIL_OP_DECR: c_int = 5;
pub const CGPU_STENCIL_OP_INCR_SAT: c_int = 6;
pub const CGPU_STENCIL_OP_DECR_SAT: c_int = 7;
pub const CGPU_STENCIL_OP_COUNT: c_int = 8;
pub const CGPU_STENCIL_OP_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUStencilOp = c_uint;
pub const ECGPUStencilOp = enum_ECGPUStencilOp;
pub const CGPU_BLEND_MODE_ADD: c_int = 0;
pub const CGPU_BLEND_MODE_SUBTRACT: c_int = 1;
pub const CGPU_BLEND_MODE_REVERSE_SUBTRACT: c_int = 2;
pub const CGPU_BLEND_MODE_MIN: c_int = 3;
pub const CGPU_BLEND_MODE_MAX: c_int = 4;
pub const CGPU_BLEND_MODE_COUNT: c_int = 5;
pub const CGPU_BLEND_MODE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUBlendMode = c_uint;
pub const ECGPUBlendMode = enum_ECGPUBlendMode;
pub const CGPU_TEX_DIMENSION_1D: c_int = 0;
pub const CGPU_TEX_DIMENSION_2D: c_int = 1;
pub const CGPU_TEX_DIMENSION_2DMS: c_int = 2;
pub const CGPU_TEX_DIMENSION_3D: c_int = 3;
pub const CGPU_TEX_DIMENSION_CUBE: c_int = 4;
pub const CGPU_TEX_DIMENSION_1D_ARRAY: c_int = 5;
pub const CGPU_TEX_DIMENSION_2D_ARRAY: c_int = 6;
pub const CGPU_TEX_DIMENSION_2DMS_ARRAY: c_int = 7;
pub const CGPU_TEX_DIMENSION_CUBE_ARRAY: c_int = 8;
pub const CGPU_TEX_DIMENSION_COUNT: c_int = 9;
pub const CGPU_TEX_DIMENSION_UNDEFINED: c_int = 10;
pub const CGPU_TEX_DIMENSION_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUTextureDimension = c_uint;
pub const ECGPUTextureDimension = enum_ECGPUTextureDimension;
pub const CGPU_SHADER_BYTECODE_TYPE_SPIRV: c_int = 0;
pub const CGPU_SHADER_BYTECODE_TYPE_DXIL: c_int = 1;
pub const CGPU_SHADER_BYTECODE_TYPE_MTL: c_int = 2;
pub const CGPU_SHADER_BYTECODE_TYPE_COUNT: c_int = 3;
pub const CGPU_SHADER_BYTECODE_TYPE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUShaderBytecodeType = c_uint;
pub const ECGPUShaderBytecodeType = enum_ECGPUShaderBytecodeType;
pub var CGPUShaderBytecodeTypeNames: [3][*c]const u8 = [3][*c]const u8{
    "spirv",
    "dxil",
    "mtl",
};
pub const CGPU_SHADER_STAGE_NONE: c_int = 0;
pub const CGPU_SHADER_STAGE_VERT: c_int = 1;
pub const CGPU_SHADER_STAGE_TESC: c_int = 2;
pub const CGPU_SHADER_STAGE_TESE: c_int = 4;
pub const CGPU_SHADER_STAGE_GEOM: c_int = 8;
pub const CGPU_SHADER_STAGE_FRAG: c_int = 16;
pub const CGPU_SHADER_STAGE_COMPUTE: c_int = 32;
pub const CGPU_SHADER_STAGE_RAYTRACING: c_int = 64;
pub const CGPU_SHADER_STAGE_ALL_GRAPHICS: c_int = 31;
pub const CGPU_SHADER_STAGE_HULL: c_int = 2;
pub const CGPU_SHADER_STAGE_DOMAIN: c_int = 4;
pub const CGPU_SHADER_STAGE_COUNT: c_int = 6;
pub const CGPU_SHADER_STAGE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUShaderStage = c_uint;
pub const ECGPUShaderStage = enum_ECGPUShaderStage;
pub const CGPUShaderStages = u32;
pub const CGPU_FENCE_STATUS_COMPLETE: c_int = 0;
pub const CGPU_FENCE_STATUS_INCOMPLETE: c_int = 1;
pub const CGPU_FENCE_STATUS_NOTSUBMITTED: c_int = 2;
pub const CGPU_FENCE_STATUS_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUFenceStatus = c_uint;
pub const ECGPUFenceStatus = enum_ECGPUFenceStatus;
pub const CGPU_QUERY_TYPE_TIMESTAMP: c_int = 0;
pub const CGPU_QUERY_TYPE_PIPELINE_STATISTICS: c_int = 1;
pub const CGPU_QUERY_TYPE_OCCLUSION: c_int = 2;
pub const CGPU_QUERY_TYPE_COUNT: c_int = 3;
pub const enum_ECGPUQueryType = c_uint;
pub const ECGPUQueryType = enum_ECGPUQueryType;
pub const CGPU_RESOURCE_STATE_UNDEFINED: c_int = 0;
pub const CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER: c_int = 1;
pub const CGPU_RESOURCE_STATE_INDEX_BUFFER: c_int = 2;
pub const CGPU_RESOURCE_STATE_RENDER_TARGET: c_int = 4;
pub const CGPU_RESOURCE_STATE_UNORDERED_ACCESS: c_int = 8;
pub const CGPU_RESOURCE_STATE_DEPTH_WRITE: c_int = 16;
pub const CGPU_RESOURCE_STATE_DEPTH_READ: c_int = 32;
pub const CGPU_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE: c_int = 64;
pub const CGPU_RESOURCE_STATE_PIXEL_SHADER_RESOURCE: c_int = 128;
pub const CGPU_RESOURCE_STATE_SHADER_RESOURCE: c_int = 192;
pub const CGPU_RESOURCE_STATE_STREAM_OUT: c_int = 256;
pub const CGPU_RESOURCE_STATE_INDIRECT_ARGUMENT: c_int = 512;
pub const CGPU_RESOURCE_STATE_COPY_DEST: c_int = 1024;
pub const CGPU_RESOURCE_STATE_COPY_SOURCE: c_int = 2048;
pub const CGPU_RESOURCE_STATE_GENERIC_READ: c_int = 2755;
pub const CGPU_RESOURCE_STATE_PRESENT: c_int = 4096;
pub const CGPU_RESOURCE_STATE_COMMON: c_int = 8192;
pub const CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE: c_int = 16384;
pub const CGPU_RESOURCE_STATE_SHADING_RATE_SOURCE: c_int = 32768;
pub const CGPU_RESOURCE_STATE_RESOLVE_DEST: c_int = 65536;
pub const CGPU_RESOURCE_STATE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUResourceState = c_uint;
pub const ECGPUResourceState = enum_ECGPUResourceState;
pub const CGPUResourceStates = u32;
pub const CGPU_MEM_USAGE_UNKNOWN: c_int = 0;
pub const CGPU_MEM_USAGE_GPU_ONLY: c_int = 1;
pub const CGPU_MEM_USAGE_CPU_ONLY: c_int = 2;
pub const CGPU_MEM_USAGE_CPU_TO_GPU: c_int = 3;
pub const CGPU_MEM_USAGE_GPU_TO_CPU: c_int = 4;
pub const CGPU_MEM_USAGE_COUNT: c_int = 5;
pub const CGPU_MEM_USAGE_MAX_ENUM: c_int = 2147483647;
pub const enum_ECGPUMemoryUsage = c_uint;
pub const ECGPUMemoryUsage = enum_ECGPUMemoryUsage;
pub const CGPU_BCF_NONE: c_int = 0;
pub const CGPU_BCF_OWN_MEMORY_BIT: c_int = 2;
pub const CGPU_BCF_PERSISTENT_MAP_BIT: c_int = 4;
pub const CGPU_BCF_ESRAM: c_int = 8;
pub const CGPU_BCF_NO_DESCRIPTOR_VIEW_CREATION: c_int = 16;
pub const CGPU_BCF_HOST_VISIBLE: c_int = 32;
pub const CGPU_BCF_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUBufferCreationFlag = c_uint;
pub const ECGPUBufferCreationFlag = enum_ECGPUBufferCreationFlag;
pub const CGPUBufferCreationFlags = u32;
pub const CGPU_TCF_NONE: c_int = 0;
pub const CGPU_TCF_OWN_MEMORY_BIT: c_int = 1;
pub const CGPU_TCF_EXPORT_BIT: c_int = 2;
pub const CGPU_TCF_EXPORT_ADAPTER_BIT: c_int = 4;
pub const CGPU_TCF_ON_TILE: c_int = 8;
pub const CGPU_TCF_NO_COMPRESSION: c_int = 16;
pub const CGPU_TCF_FORCE_2D: c_int = 32;
pub const CGPU_TCF_FORCE_3D: c_int = 64;
pub const CGPU_TCF_ALLOW_DISPLAY_TARGET: c_int = 128;
pub const CGPU_TCF_NORMAL_MAP: c_int = 256;
pub const CGPU_TCF_FRAG_MASK: c_int = 512;
pub const CGPU_TCF_USABLE_MAX: c_int = 262144;
pub const CGPU_TCF_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUTextureCreationFlag = c_uint;
pub const ECGPUTextureCreationFlag = enum_ECGPUTextureCreationFlag;
pub const CGPUTextureCreationFlags = u32;
pub const CGPU_SAMPLE_COUNT_1: c_int = 1;
pub const CGPU_SAMPLE_COUNT_2: c_int = 2;
pub const CGPU_SAMPLE_COUNT_4: c_int = 4;
pub const CGPU_SAMPLE_COUNT_8: c_int = 8;
pub const CGPU_SAMPLE_COUNT_16: c_int = 16;
pub const CGPU_SAMPLE_COUNT_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUSampleCount = c_uint;
pub const ECGPUSampleCount = enum_ECGPUSampleCount;
pub const CGPU_PIPELINE_TYPE_NONE: c_int = 0;
pub const CGPU_PIPELINE_TYPE_COMPUTE: c_int = 1;
pub const CGPU_PIPELINE_TYPE_GRAPHICS: c_int = 2;
pub const CGPU_PIPELINE_TYPE_RAYTRACING: c_int = 3;
pub const CGPU_PIPELINE_TYPE_COUNT: c_int = 4;
pub const CGPU_PIPELINE_TYPE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUPipelineType = c_uint;
pub const ECGPUPipelineType = enum_ECGPUPipelineType;
pub const CGPU_RESOURCE_TYPE_NONE: c_int = 0;
pub const CGPU_RESOURCE_TYPE_SAMPLER: c_int = 1;
pub const CGPU_RESOURCE_TYPE_TEXTURE: c_int = 2;
pub const CGPU_RESOURCE_TYPE_RENDER_TARGET: c_int = 4;
pub const CGPU_RESOURCE_TYPE_DEPTH_STENCIL: c_int = 8;
pub const CGPU_RESOURCE_TYPE_RW_TEXTURE: c_int = 16;
pub const CGPU_RESOURCE_TYPE_BUFFER: c_int = 32;
pub const CGPU_RESOURCE_TYPE_BUFFER_RAW: c_int = 96;
pub const CGPU_RESOURCE_TYPE_RW_BUFFER: c_int = 128;
pub const CGPU_RESOURCE_TYPE_RW_BUFFER_RAW: c_int = 384;
pub const CGPU_RESOURCE_TYPE_UNIFORM_BUFFER: c_int = 512;
pub const CGPU_RESOURCE_TYPE_PUSH_CONSTANT: c_int = 1024;
pub const CGPU_RESOURCE_TYPE_VERTEX_BUFFER: c_int = 2048;
pub const CGPU_RESOURCE_TYPE_INDEX_BUFFER: c_int = 4096;
pub const CGPU_RESOURCE_TYPE_INDIRECT_BUFFER: c_int = 8192;
pub const CGPU_RESOURCE_TYPE_TEXTURE_CUBE: c_int = 16386;
pub const CGPU_RESOURCE_TYPE_RENDER_TARGET_MIP_SLICES: c_int = 32768;
pub const CGPU_RESOURCE_TYPE_RENDER_TARGET_ARRAY_SLICES: c_int = 65536;
pub const CGPU_RESOURCE_TYPE_RENDER_TARGET_DEPTH_SLICES: c_int = 131072;
pub const CGPU_RESOURCE_TYPE_RAY_TRACING: c_int = 262144;
pub const CGPU_RESOURCE_TYPE_INPUT_ATTACHMENT: c_int = 524288;
pub const CGPU_RESOURCE_TYPE_TEXEL_BUFFER: c_int = 1048576;
pub const CGPU_RESOURCE_TYPE_RW_TEXEL_BUFFER: c_int = 2097152;
pub const CGPU_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER: c_int = 4194304;
pub const CGPU_RESOURCE_TYPE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUResourceType = c_uint;
pub const ECGPUResourceType = enum_ECGPUResourceType;
pub const CGPUResourceTypes = u32;
pub const CGPU_TVU_SRV: c_int = 1;
pub const CGPU_TVU_RTV_DSV: c_int = 2;
pub const CGPU_TVU_UAV: c_int = 4;
pub const CGPU_TVU_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUTexutreViewUsage = c_uint;
pub const ECGPUTexutreViewUsage = enum_ECGPUTexutreViewUsage;
pub const CGPUTexutreViewUsages = u32;
pub const CGPU_TVA_COLOR: c_int = 1;
pub const CGPU_TVA_DEPTH: c_int = 2;
pub const CGPU_TVA_STENCIL: c_int = 4;
pub const CGPU_TVA_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUTextureViewAspect = c_uint;
pub const ECGPUTextureViewAspect = enum_ECGPUTextureViewAspect;
pub const CGPUTextureViewAspects = u32;
pub const CGPU_SHADING_RATE_FULL: c_int = 0;
pub const CGPU_SHADING_RATE_HALF: c_int = 1;
pub const CGPU_SHADING_RATE_QUARTER: c_int = 2;
pub const CGPU_SHADING_RATE_1X2: c_int = 3;
pub const CGPU_SHADING_RATE_2X1: c_int = 4;
pub const CGPU_SHADING_RATE_2X4: c_int = 5;
pub const CGPU_SHADING_RATE_4X2: c_int = 6;
pub const CGPU_SHADING_RATE_COUNT: c_int = 7;
pub const CGPU_SHADING_RATE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUShadingRate = c_uint;
pub const ECGPUShadingRate = enum_ECGPUShadingRate;
pub const CGPU_SHADING_RATE_COMBINER_PASSTHROUGH: c_int = 0;
pub const CGPU_SHADING_RATE_COMBINER_OVERRIDE: c_int = 1;
pub const CGPU_SHADING_RATE_COMBINER_MIN: c_int = 2;
pub const CGPU_SHADING_RATE_COMBINER_MAX: c_int = 3;
pub const CGPU_SHADING_RATE_COMBINER_SUM: c_int = 4;
pub const CGPU_SHADING_RATE_COMBINER_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUShadingRateCombiner = c_uint;
pub const ECGPUShadingRateCombiner = enum_ECGPUShadingRateCombiner;
pub const CGPU_DSTORAGE_AVAILABILITY_NONE: c_int = 0;
pub const CGPU_DSTORAGE_AVAILABILITY_HARDWARE: c_int = 1;
pub const CGPU_DSTORAGE_AVAILABILITY_SOFTWARE: c_int = 2;
pub const CGPU_DSTORAGE_AVAILABILITY_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUDStorageAvailability = c_uint;
pub const ECGPUDStorageAvailability = enum_ECGPUDStorageAvailability;
pub const CGPU_DSTORAGE_SOURCE_FILE: c_int = 0;
pub const CGPU_DSTORAGE_SOURCE_MEMORY: c_int = 1;
pub const CGPU_DSTORAGE_SOURCE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUDStorageSource = c_uint;
pub const ECGPUDStorageSource = enum_ECGPUDStorageSource;
pub const CGPU_DSTORAGE_PRIORITY_LOW: c_int = -1;
pub const CGPU_DSTORAGE_PRIORITY_NORMAL: c_int = 0;
pub const CGPU_DSTORAGE_PRIORITY_HIGH: c_int = 1;
pub const CGPU_DSTORAGE_PRIORITY_REALTIME: c_int = 2;
pub const CGPU_DSTORAGE_PRIORITY_COUNT: c_int = 4;
pub const CGPU_DSTORAGE_PRIORITY_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUDStoragePriority = c_int;
pub const ECGPUDStoragePriority = enum_ECGPUDStoragePriority;
pub const CGPU_DSTORAGE_COMPRESSION_NONE: c_int = 0;
pub const CGPU_DSTORAGE_COMPRESSION_CUSTOM: c_int = 128;
pub const CGPU_DSTORAGE_COMPRESSION_MAX_ENUM_BIT: c_int = 255;
pub const enum_ECGPUDStorageCompression = c_uint;
pub const ECGPUDStorageCompression = enum_ECGPUDStorageCompression;
pub const CGPUDStorageCompression = u8;
pub inline fn FormatUtil_IsDepthStencilFormat(fmt: ECGPUFormat) bool {
    while (true) {
        switch (fmt) {
            @bitCast(c_uint, @as(c_int, 134)), @bitCast(c_uint, @as(c_int, 135)), @bitCast(c_uint, @as(c_int, 131)), @bitCast(c_uint, @as(c_int, 130)), @bitCast(c_uint, @as(c_int, 129)), @bitCast(c_uint, @as(c_int, 133)) => return @as(c_int, 1) != 0,
            else => return @as(c_int, 0) != 0,
        }
        break;
    }
    return @as(c_int, 0) != 0;
}
pub inline fn FormatUtil_IsDepthOnlyFormat(fmt: ECGPUFormat) bool {
    while (true) {
        switch (fmt) {
            @bitCast(c_uint, @as(c_int, 131)), @bitCast(c_uint, @as(c_int, 129)) => return @as(c_int, 1) != 0,
            else => return @as(c_int, 0) != 0,
        }
        break;
    }
    return @as(c_int, 0) != 0;
}
pub inline fn FormatUtil_BitSizeOfBlock(fmt: ECGPUFormat) u32 {
    while (true) {
        switch (fmt) {
            @bitCast(c_uint, @as(c_int, 0)) => return 0,
            @bitCast(c_uint, @as(c_int, 1)) => return 8,
            @bitCast(c_uint, @as(c_int, 2)) => return 8,
            @bitCast(c_uint, @as(c_int, 3)) => return 8,
            @bitCast(c_uint, @as(c_int, 4)) => return 8,
            @bitCast(c_uint, @as(c_int, 5)) => return 8,
            @bitCast(c_uint, @as(c_int, 6)) => return 8,
            @bitCast(c_uint, @as(c_int, 7)) => return 8,
            @bitCast(c_uint, @as(c_int, 8)) => return 8,
            @bitCast(c_uint, @as(c_int, 9)) => return 8,
            @bitCast(c_uint, @as(c_int, 10)) => return 8,
            @bitCast(c_uint, @as(c_int, 11)) => return 8,
            @bitCast(c_uint, @as(c_int, 12)) => return 8,
            @bitCast(c_uint, @as(c_int, 13)) => return 16,
            @bitCast(c_uint, @as(c_int, 14)) => return 16,
            @bitCast(c_uint, @as(c_int, 15)) => return 16,
            @bitCast(c_uint, @as(c_int, 16)) => return 16,
            @bitCast(c_uint, @as(c_int, 17)) => return 16,
            @bitCast(c_uint, @as(c_int, 18)) => return 16,
            @bitCast(c_uint, @as(c_int, 19)) => return 16,
            @bitCast(c_uint, @as(c_int, 20)) => return 16,
            @bitCast(c_uint, @as(c_int, 21)) => return 16,
            @bitCast(c_uint, @as(c_int, 22)) => return 16,
            @bitCast(c_uint, @as(c_int, 23)) => return 16,
            @bitCast(c_uint, @as(c_int, 24)) => return 16,
            @bitCast(c_uint, @as(c_int, 25)) => return 16,
            @bitCast(c_uint, @as(c_int, 26)) => return 16,
            @bitCast(c_uint, @as(c_int, 27)) => return 16,
            @bitCast(c_uint, @as(c_int, 28)) => return 16,
            @bitCast(c_uint, @as(c_int, 29)) => return 16,
            @bitCast(c_uint, @as(c_int, 30)) => return 16,
            @bitCast(c_uint, @as(c_int, 31)) => return 16,
            @bitCast(c_uint, @as(c_int, 32)) => return 16,
            @bitCast(c_uint, @as(c_int, 33)) => return 16,
            @bitCast(c_uint, @as(c_int, 34)) => return 16,
            @bitCast(c_uint, @as(c_int, 35)) => return 16,
            @bitCast(c_uint, @as(c_int, 36)) => return 16,
            @bitCast(c_uint, @as(c_int, 37)) => return 16,
            @bitCast(c_uint, @as(c_int, 38)) => return 16,
            @bitCast(c_uint, @as(c_int, 39)) => return 16,
            @bitCast(c_uint, @as(c_int, 40)) => return 16,
            @bitCast(c_uint, @as(c_int, 41)) => return 16,
            @bitCast(c_uint, @as(c_int, 42)) => return 16,
            @bitCast(c_uint, @as(c_int, 43)) => return 16,
            @bitCast(c_uint, @as(c_int, 44)) => return 16,
            @bitCast(c_uint, @as(c_int, 45)) => return 24,
            @bitCast(c_uint, @as(c_int, 46)) => return 24,
            @bitCast(c_uint, @as(c_int, 47)) => return 24,
            @bitCast(c_uint, @as(c_int, 48)) => return 24,
            @bitCast(c_uint, @as(c_int, 49)) => return 24,
            @bitCast(c_uint, @as(c_int, 50)) => return 24,
            @bitCast(c_uint, @as(c_int, 51)) => return 24,
            @bitCast(c_uint, @as(c_int, 52)) => return 24,
            @bitCast(c_uint, @as(c_int, 53)) => return 24,
            @bitCast(c_uint, @as(c_int, 54)) => return 24,
            @bitCast(c_uint, @as(c_int, 96)) => return 48,
            @bitCast(c_uint, @as(c_int, 97)) => return 48,
            @bitCast(c_uint, @as(c_int, 98)) => return 48,
            @bitCast(c_uint, @as(c_int, 99)) => return 48,
            @bitCast(c_uint, @as(c_int, 100)) => return 48,
            @bitCast(c_uint, @as(c_int, 101)) => return 48,
            @bitCast(c_uint, @as(c_int, 102)) => return 64,
            @bitCast(c_uint, @as(c_int, 103)) => return 64,
            @bitCast(c_uint, @as(c_int, 104)) => return 64,
            @bitCast(c_uint, @as(c_int, 105)) => return 64,
            @bitCast(c_uint, @as(c_int, 106)) => return 64,
            @bitCast(c_uint, @as(c_int, 107)) => return 64,
            @bitCast(c_uint, @as(c_int, 108)) => return 64,
            @bitCast(c_uint, @as(c_int, 109)) => return 64,
            @bitCast(c_uint, @as(c_int, 110)) => return 64,
            @bitCast(c_uint, @as(c_int, 111)) => return 96,
            @bitCast(c_uint, @as(c_int, 112)) => return 96,
            @bitCast(c_uint, @as(c_int, 113)) => return 96,
            @bitCast(c_uint, @as(c_int, 114)) => return 128,
            @bitCast(c_uint, @as(c_int, 115)) => return 128,
            @bitCast(c_uint, @as(c_int, 116)) => return 128,
            @bitCast(c_uint, @as(c_int, 117)) => return 64,
            @bitCast(c_uint, @as(c_int, 118)) => return 64,
            @bitCast(c_uint, @as(c_int, 119)) => return 64,
            @bitCast(c_uint, @as(c_int, 120)) => return 128,
            @bitCast(c_uint, @as(c_int, 121)) => return 128,
            @bitCast(c_uint, @as(c_int, 122)) => return 128,
            @bitCast(c_uint, @as(c_int, 123)) => return 192,
            @bitCast(c_uint, @as(c_int, 124)) => return 192,
            @bitCast(c_uint, @as(c_int, 125)) => return 192,
            @bitCast(c_uint, @as(c_int, 126)) => return 256,
            @bitCast(c_uint, @as(c_int, 127)) => return 256,
            @bitCast(c_uint, @as(c_int, 128)) => return 256,
            @bitCast(c_uint, @as(c_int, 129)) => return 16,
            @bitCast(c_uint, @as(c_int, 132)) => return 8,
            @bitCast(c_uint, @as(c_int, 135)) => return 64,
            @bitCast(c_uint, @as(c_int, 136)) => return 64,
            @bitCast(c_uint, @as(c_int, 137)) => return 64,
            @bitCast(c_uint, @as(c_int, 138)) => return 64,
            @bitCast(c_uint, @as(c_int, 139)) => return 64,
            @bitCast(c_uint, @as(c_int, 140)) => return 128,
            @bitCast(c_uint, @as(c_int, 141)) => return 128,
            @bitCast(c_uint, @as(c_int, 142)) => return 128,
            @bitCast(c_uint, @as(c_int, 143)) => return 128,
            @bitCast(c_uint, @as(c_int, 144)) => return 64,
            @bitCast(c_uint, @as(c_int, 145)) => return 64,
            @bitCast(c_uint, @as(c_int, 146)) => return 128,
            @bitCast(c_uint, @as(c_int, 147)) => return 128,
            @bitCast(c_uint, @as(c_int, 148)) => return 128,
            @bitCast(c_uint, @as(c_int, 149)) => return 128,
            @bitCast(c_uint, @as(c_int, 150)) => return 128,
            @bitCast(c_uint, @as(c_int, 151)) => return 128,
            @bitCast(c_uint, @as(c_int, 152)) => return 64,
            @bitCast(c_uint, @as(c_int, 153)) => return 64,
            @bitCast(c_uint, @as(c_int, 154)) => return 64,
            @bitCast(c_uint, @as(c_int, 155)) => return 64,
            @bitCast(c_uint, @as(c_int, 156)) => return 64,
            @bitCast(c_uint, @as(c_int, 157)) => return 64,
            @bitCast(c_uint, @as(c_int, 158)) => return 64,
            @bitCast(c_uint, @as(c_int, 159)) => return 64,
            @bitCast(c_uint, @as(c_int, 160)) => return 64,
            @bitCast(c_uint, @as(c_int, 161)) => return 64,
            @bitCast(c_uint, @as(c_int, 162)) => return 64,
            @bitCast(c_uint, @as(c_int, 163)) => return 64,
            @bitCast(c_uint, @as(c_int, 164)) => return 64,
            @bitCast(c_uint, @as(c_int, 165)) => return 64,
            @bitCast(c_uint, @as(c_int, 166)) => return 64,
            @bitCast(c_uint, @as(c_int, 167)) => return 64,
            @bitCast(c_uint, @as(c_int, 168)) => return 64,
            @bitCast(c_uint, @as(c_int, 169)) => return 64,
            @bitCast(c_uint, @as(c_int, 170)) => return 128,
            @bitCast(c_uint, @as(c_int, 171)) => return 128,
            @bitCast(c_uint, @as(c_int, 172)) => return 128,
            @bitCast(c_uint, @as(c_int, 173)) => return 128,
            @bitCast(c_uint, @as(c_int, 174)) => return 128,
            @bitCast(c_uint, @as(c_int, 175)) => return 128,
            @bitCast(c_uint, @as(c_int, 176)) => return 128,
            @bitCast(c_uint, @as(c_int, 177)) => return 128,
            @bitCast(c_uint, @as(c_int, 178)) => return 128,
            @bitCast(c_uint, @as(c_int, 179)) => return 128,
            @bitCast(c_uint, @as(c_int, 180)) => return 128,
            @bitCast(c_uint, @as(c_int, 181)) => return 128,
            @bitCast(c_uint, @as(c_int, 182)) => return 128,
            @bitCast(c_uint, @as(c_int, 183)) => return 128,
            @bitCast(c_uint, @as(c_int, 184)) => return 128,
            @bitCast(c_uint, @as(c_int, 185)) => return 128,
            @bitCast(c_uint, @as(c_int, 186)) => return 128,
            @bitCast(c_uint, @as(c_int, 187)) => return 128,
            @bitCast(c_uint, @as(c_int, 188)) => return 128,
            @bitCast(c_uint, @as(c_int, 189)) => return 128,
            @bitCast(c_uint, @as(c_int, 190)) => return 128,
            @bitCast(c_uint, @as(c_int, 191)) => return 128,
            @bitCast(c_uint, @as(c_int, 192)) => return 128,
            @bitCast(c_uint, @as(c_int, 193)) => return 128,
            @bitCast(c_uint, @as(c_int, 194)) => return 128,
            @bitCast(c_uint, @as(c_int, 195)) => return 128,
            @bitCast(c_uint, @as(c_int, 196)) => return 128,
            @bitCast(c_uint, @as(c_int, 197)) => return 128,
            @bitCast(c_uint, @as(c_int, 198)) => return 8,
            @bitCast(c_uint, @as(c_int, 199)) => return 8,
            @bitCast(c_uint, @as(c_int, 200)) => return 8,
            @bitCast(c_uint, @as(c_int, 201)) => return 16,
            @bitCast(c_uint, @as(c_int, 209)) => return 8,
            @bitCast(c_uint, @as(c_int, 210)) => return 8,
            @bitCast(c_uint, @as(c_int, 211)) => return 8,
            @bitCast(c_uint, @as(c_int, 212)) => return 8,
            @bitCast(c_uint, @as(c_int, 213)) => return 8,
            @bitCast(c_uint, @as(c_int, 214)) => return 8,
            @bitCast(c_uint, @as(c_int, 215)) => return 8,
            @bitCast(c_uint, @as(c_int, 216)) => return 8,
            @bitCast(c_uint, @as(c_int, 217)) => return 4,
            @bitCast(c_uint, @as(c_int, 218)) => return 4,
            else => return 32,
        }
        break;
    }
    return 0;
}
pub inline fn FormatUtil_WidthOfBlock(fmt: ECGPUFormat) u32 {
    while (true) {
        switch (fmt) {
            @bitCast(c_uint, @as(c_int, 0)) => return 1,
            @bitCast(c_uint, @as(c_int, 1)) => return 8,
            @bitCast(c_uint, @as(c_int, 2)) => return 4,
            @bitCast(c_uint, @as(c_int, 3)) => return 2,
            @bitCast(c_uint, @as(c_int, 136)) => return 4,
            @bitCast(c_uint, @as(c_int, 137)) => return 4,
            @bitCast(c_uint, @as(c_int, 138)) => return 4,
            @bitCast(c_uint, @as(c_int, 139)) => return 4,
            @bitCast(c_uint, @as(c_int, 140)) => return 4,
            @bitCast(c_uint, @as(c_int, 141)) => return 4,
            @bitCast(c_uint, @as(c_int, 142)) => return 4,
            @bitCast(c_uint, @as(c_int, 143)) => return 4,
            @bitCast(c_uint, @as(c_int, 144)) => return 4,
            @bitCast(c_uint, @as(c_int, 145)) => return 4,
            @bitCast(c_uint, @as(c_int, 146)) => return 4,
            @bitCast(c_uint, @as(c_int, 147)) => return 4,
            @bitCast(c_uint, @as(c_int, 148)) => return 4,
            @bitCast(c_uint, @as(c_int, 149)) => return 4,
            @bitCast(c_uint, @as(c_int, 150)) => return 4,
            @bitCast(c_uint, @as(c_int, 151)) => return 4,
            @bitCast(c_uint, @as(c_int, 152)) => return 8,
            @bitCast(c_uint, @as(c_int, 153)) => return 4,
            @bitCast(c_uint, @as(c_int, 154)) => return 8,
            @bitCast(c_uint, @as(c_int, 155)) => return 4,
            @bitCast(c_uint, @as(c_int, 156)) => return 8,
            @bitCast(c_uint, @as(c_int, 157)) => return 4,
            @bitCast(c_uint, @as(c_int, 158)) => return 8,
            @bitCast(c_uint, @as(c_int, 159)) => return 4,
            @bitCast(c_uint, @as(c_int, 160)) => return 4,
            @bitCast(c_uint, @as(c_int, 161)) => return 4,
            @bitCast(c_uint, @as(c_int, 162)) => return 4,
            @bitCast(c_uint, @as(c_int, 163)) => return 4,
            @bitCast(c_uint, @as(c_int, 164)) => return 4,
            @bitCast(c_uint, @as(c_int, 165)) => return 4,
            @bitCast(c_uint, @as(c_int, 166)) => return 4,
            @bitCast(c_uint, @as(c_int, 167)) => return 4,
            @bitCast(c_uint, @as(c_int, 168)) => return 4,
            @bitCast(c_uint, @as(c_int, 169)) => return 4,
            @bitCast(c_uint, @as(c_int, 170)) => return 4,
            @bitCast(c_uint, @as(c_int, 171)) => return 4,
            @bitCast(c_uint, @as(c_int, 172)) => return 5,
            @bitCast(c_uint, @as(c_int, 173)) => return 5,
            @bitCast(c_uint, @as(c_int, 174)) => return 5,
            @bitCast(c_uint, @as(c_int, 175)) => return 5,
            @bitCast(c_uint, @as(c_int, 176)) => return 6,
            @bitCast(c_uint, @as(c_int, 177)) => return 6,
            @bitCast(c_uint, @as(c_int, 178)) => return 6,
            @bitCast(c_uint, @as(c_int, 179)) => return 6,
            @bitCast(c_uint, @as(c_int, 180)) => return 8,
            @bitCast(c_uint, @as(c_int, 181)) => return 8,
            @bitCast(c_uint, @as(c_int, 182)) => return 8,
            @bitCast(c_uint, @as(c_int, 183)) => return 8,
            @bitCast(c_uint, @as(c_int, 184)) => return 8,
            @bitCast(c_uint, @as(c_int, 185)) => return 8,
            @bitCast(c_uint, @as(c_int, 186)) => return 10,
            @bitCast(c_uint, @as(c_int, 187)) => return 10,
            @bitCast(c_uint, @as(c_int, 188)) => return 10,
            @bitCast(c_uint, @as(c_int, 189)) => return 10,
            @bitCast(c_uint, @as(c_int, 190)) => return 10,
            @bitCast(c_uint, @as(c_int, 191)) => return 10,
            @bitCast(c_uint, @as(c_int, 192)) => return 10,
            @bitCast(c_uint, @as(c_int, 193)) => return 10,
            @bitCast(c_uint, @as(c_int, 194)) => return 12,
            @bitCast(c_uint, @as(c_int, 195)) => return 12,
            @bitCast(c_uint, @as(c_int, 196)) => return 12,
            @bitCast(c_uint, @as(c_int, 197)) => return 12,
            @bitCast(c_uint, @as(c_int, 198)) => return 2,
            else => return 1,
        }
        break;
    }
    return 0;
}
pub inline fn FormatUtil_HeightOfBlock(fmt: ECGPUFormat) u32 {
    while (true) {
        switch (fmt) {
            @bitCast(c_uint, @as(c_int, 0)) => return 1,
            @bitCast(c_uint, @as(c_int, 136)) => return 4,
            @bitCast(c_uint, @as(c_int, 137)) => return 4,
            @bitCast(c_uint, @as(c_int, 138)) => return 4,
            @bitCast(c_uint, @as(c_int, 139)) => return 4,
            @bitCast(c_uint, @as(c_int, 140)) => return 4,
            @bitCast(c_uint, @as(c_int, 141)) => return 4,
            @bitCast(c_uint, @as(c_int, 142)) => return 4,
            @bitCast(c_uint, @as(c_int, 143)) => return 4,
            @bitCast(c_uint, @as(c_int, 144)) => return 4,
            @bitCast(c_uint, @as(c_int, 145)) => return 4,
            @bitCast(c_uint, @as(c_int, 146)) => return 4,
            @bitCast(c_uint, @as(c_int, 147)) => return 4,
            @bitCast(c_uint, @as(c_int, 148)) => return 4,
            @bitCast(c_uint, @as(c_int, 149)) => return 4,
            @bitCast(c_uint, @as(c_int, 150)) => return 4,
            @bitCast(c_uint, @as(c_int, 151)) => return 4,
            @bitCast(c_uint, @as(c_int, 152)) => return 4,
            @bitCast(c_uint, @as(c_int, 153)) => return 4,
            @bitCast(c_uint, @as(c_int, 154)) => return 4,
            @bitCast(c_uint, @as(c_int, 155)) => return 4,
            @bitCast(c_uint, @as(c_int, 156)) => return 4,
            @bitCast(c_uint, @as(c_int, 157)) => return 4,
            @bitCast(c_uint, @as(c_int, 158)) => return 4,
            @bitCast(c_uint, @as(c_int, 159)) => return 4,
            @bitCast(c_uint, @as(c_int, 160)) => return 4,
            @bitCast(c_uint, @as(c_int, 161)) => return 4,
            @bitCast(c_uint, @as(c_int, 162)) => return 4,
            @bitCast(c_uint, @as(c_int, 163)) => return 4,
            @bitCast(c_uint, @as(c_int, 164)) => return 4,
            @bitCast(c_uint, @as(c_int, 165)) => return 4,
            @bitCast(c_uint, @as(c_int, 166)) => return 4,
            @bitCast(c_uint, @as(c_int, 167)) => return 4,
            @bitCast(c_uint, @as(c_int, 168)) => return 4,
            @bitCast(c_uint, @as(c_int, 169)) => return 4,
            @bitCast(c_uint, @as(c_int, 170)) => return 4,
            @bitCast(c_uint, @as(c_int, 171)) => return 4,
            @bitCast(c_uint, @as(c_int, 172)) => return 4,
            @bitCast(c_uint, @as(c_int, 173)) => return 4,
            @bitCast(c_uint, @as(c_int, 174)) => return 5,
            @bitCast(c_uint, @as(c_int, 175)) => return 5,
            @bitCast(c_uint, @as(c_int, 176)) => return 5,
            @bitCast(c_uint, @as(c_int, 177)) => return 5,
            @bitCast(c_uint, @as(c_int, 178)) => return 6,
            @bitCast(c_uint, @as(c_int, 179)) => return 6,
            @bitCast(c_uint, @as(c_int, 180)) => return 5,
            @bitCast(c_uint, @as(c_int, 181)) => return 5,
            @bitCast(c_uint, @as(c_int, 182)) => return 6,
            @bitCast(c_uint, @as(c_int, 183)) => return 6,
            @bitCast(c_uint, @as(c_int, 184)) => return 8,
            @bitCast(c_uint, @as(c_int, 185)) => return 8,
            @bitCast(c_uint, @as(c_int, 186)) => return 5,
            @bitCast(c_uint, @as(c_int, 187)) => return 5,
            @bitCast(c_uint, @as(c_int, 188)) => return 6,
            @bitCast(c_uint, @as(c_int, 189)) => return 6,
            @bitCast(c_uint, @as(c_int, 190)) => return 8,
            @bitCast(c_uint, @as(c_int, 191)) => return 8,
            @bitCast(c_uint, @as(c_int, 192)) => return 10,
            @bitCast(c_uint, @as(c_int, 193)) => return 10,
            @bitCast(c_uint, @as(c_int, 194)) => return 10,
            @bitCast(c_uint, @as(c_int, 195)) => return 10,
            @bitCast(c_uint, @as(c_int, 196)) => return 12,
            @bitCast(c_uint, @as(c_int, 197)) => return 12,
            else => return 1,
        }
        break;
    }
    return 0;
}
pub const CGPUQueueIndex = u32;
pub const struct_CGPUSurfaceDescriptor = opaque {};
pub const struct_CGPUSurface = opaque {};
pub const CGPUSurfaceId = ?*const struct_CGPUSurface;
pub const CGPU_BACKEND_VULKAN: c_int = 0;
pub const CGPU_BACKEND_D3D12: c_int = 1;
pub const CGPU_BACKEND_XBOX_D3D12: c_int = 2;
pub const CGPU_BACKEND_AGC: c_int = 3;
pub const CGPU_BACKEND_METAL: c_int = 4;
pub const CGPU_BACKEND_COUNT: c_int = 5;
pub const CGPU_BACKEND_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUBackend = c_uint;
pub const ECGPUBackend = enum_ECGPUBackend;
pub const struct_CGPUChainedDescriptor = extern struct {
    backend: ECGPUBackend,
};
pub const CGPUChainedDescriptor = struct_CGPUChainedDescriptor;
pub const struct_CGPUInstanceDescriptor = extern struct {
    chained: [*c]const CGPUChainedDescriptor,
    backend: ECGPUBackend,
    enable_debug_layer: bool,
    enable_gpu_based_validation: bool,
    enable_set_name: bool,
};
pub const CGPUInstanceId = [*c]const struct_CGPUInstance;
pub const CGPUProcCreateInstance = ?fn ([*c]const struct_CGPUInstanceDescriptor) callconv(.C) CGPUInstanceId;
pub const struct_CGPUInstanceFeatures = extern struct {
    specialization_constant: bool,
};
pub const CGPUProcQueryInstanceFeatures = ?fn (CGPUInstanceId, [*c]struct_CGPUInstanceFeatures) callconv(.C) void;
pub const CGPUProcFreeInstance = ?fn (CGPUInstanceId) callconv(.C) void;
pub const struct_CGPUAdapter = extern struct {
    instance: CGPUInstanceId,
    proc_table_cache: [*c]const CGPUProcTable,
};
pub const CGPUAdapterId = [*c]const struct_CGPUAdapter;
pub const CGPUProcEnumAdapters = ?fn (CGPUInstanceId, [*c]CGPUAdapterId, [*c]u32) callconv(.C) void; // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu\api.h:620:10: warning: struct demoted to opaque type - has bitfield
pub const struct_CGPUAdapterDetail = opaque {};
pub const CGPUProcQueryAdapterDetail = ?fn (CGPUAdapterId) callconv(.C) ?*const struct_CGPUAdapterDetail;
pub const struct_CGPUDevice = extern struct {
    adapter: CGPUAdapterId,
    proc_table_cache: [*c]const CGPUProcTable,
    next_texture_id: u64,
    is_lost: bool,
};
pub const CGPUDeviceId = [*c]const struct_CGPUDevice;
pub const CGPUProcQueryVideoMemoryInfo = ?fn (CGPUDeviceId, [*c]u64, [*c]u64) callconv(.C) void;
pub const CGPUProcQuerySharedMemoryInfo = ?fn (CGPUDeviceId, [*c]u64, [*c]u64) callconv(.C) void;
pub const CGPU_QUEUE_TYPE_GRAPHICS: c_int = 0;
pub const CGPU_QUEUE_TYPE_COMPUTE: c_int = 1;
pub const CGPU_QUEUE_TYPE_TRANSFER: c_int = 2;
pub const CGPU_QUEUE_TYPE_COUNT: c_int = 3;
pub const CGPU_QUEUE_TYPE_MAX_ENUM_BIT: c_int = 2147483647;
pub const enum_ECGPUQueueType = c_uint;
pub const ECGPUQueueType = enum_ECGPUQueueType;
pub const CGPUProcQueryQueueCount = ?fn (CGPUAdapterId, ECGPUQueueType) callconv(.C) u32;
pub const struct_CGPUQueueGroupDescriptor = extern struct {
    queue_type: ECGPUQueueType,
    queue_count: u32,
};
pub const CGPUQueueGroupDescriptor = struct_CGPUQueueGroupDescriptor;
pub const struct_CGPUDeviceDescriptor = extern struct {
    disable_pipeline_cache: bool,
    queue_groups: [*c]CGPUQueueGroupDescriptor,
    queue_group_count: u32,
};
pub const CGPUProcCreateDevice = ?fn (CGPUAdapterId, [*c]const struct_CGPUDeviceDescriptor) callconv(.C) CGPUDeviceId;
pub const CGPUProcFreeDevice = ?fn (CGPUDeviceId) callconv(.C) void;
pub const struct_CGPUFence = extern struct {
    device: CGPUDeviceId,
};
pub const CGPUFenceId = [*c]const struct_CGPUFence;
pub const CGPUProcCreateFence = ?fn (CGPUDeviceId) callconv(.C) CGPUFenceId;
pub const CGPUProcWaitFences = ?fn ([*c]const CGPUFenceId, u32) callconv(.C) void;
pub const CGPUProcQueryFenceStatus = ?fn (CGPUFenceId) callconv(.C) ECGPUFenceStatus;
pub const CGPUProcFreeFence = ?fn (CGPUFenceId) callconv(.C) void;
pub const struct_CGPUSemaphore = extern struct {
    device: CGPUDeviceId,
};
pub const CGPUSemaphoreId = [*c]const struct_CGPUSemaphore;
pub const CGPUProcCreateSemaphore = ?fn (CGPUDeviceId) callconv(.C) CGPUSemaphoreId;
pub const CGPUProcFreeSemaphore = ?fn (CGPUSemaphoreId) callconv(.C) void;
pub const struct_CGPURootSignaturePoolDescriptor = extern struct {
    name: [*c]const char8_t,
};
pub const struct_CGPURootSignaturePool = extern struct {
    device: CGPUDeviceId,
    pipeline_type: ECGPUPipelineType,
};
pub const CGPURootSignaturePoolId = [*c]const struct_CGPURootSignaturePool;
pub const CGPUProcCreateRootSignaturePool = ?fn (CGPUDeviceId, [*c]const struct_CGPURootSignaturePoolDescriptor) callconv(.C) CGPURootSignaturePoolId;
pub const CGPUProcFreeRootSignaturePool = ?fn (CGPURootSignaturePoolId) callconv(.C) void;
pub const struct_CGPUVertexInput = extern struct {
    name: [*c]const char8_t,
    semantics: [*c]const char8_t,
    format: ECGPUFormat,
};
pub const CGPUVertexInput = struct_CGPUVertexInput;
pub const struct_CGPUShaderResource = extern struct {
    name: [*c]const char8_t,
    name_hash: u64,
    type: ECGPUResourceType,
    dim: ECGPUTextureDimension,
    set: u32,
    binding: u32,
    size: u32,
    offset: u32,
    stages: CGPUShaderStages,
};
pub const CGPUShaderResource = struct_CGPUShaderResource;
pub const struct_CGPUShaderReflection = extern struct {
    entry_name: [*c]const char8_t,
    stage: ECGPUShaderStage,
    vertex_inputs: [*c]CGPUVertexInput,
    shader_resources: [*c]CGPUShaderResource,
    vertex_inputs_count: u32,
    shader_resources_count: u32,
    thread_group_sizes: [3]u32,
};
pub const CGPUShaderReflection = struct_CGPUShaderReflection;
pub const struct_CGPUShaderLibrary = extern struct {
    device: CGPUDeviceId,
    name: [*c]char8_t,
    entry_reflections: [*c]CGPUShaderReflection,
    entrys_count: u32,
};
pub const CGPUShaderLibraryId = [*c]const struct_CGPUShaderLibrary;
const union_unnamed_1 = extern union {
    u: u64,
    i: i64,
    f: f64,
};
pub const struct_CGPUConstantSpecialization = extern struct {
    constantID: u32,
    unnamed_0: union_unnamed_1,
};
pub const CGPUConstantSpecialization = struct_CGPUConstantSpecialization;
pub const struct_CGPUShaderEntryDescriptor = extern struct {
    library: CGPUShaderLibraryId,
    entry: [*c]const char8_t,
    stage: ECGPUShaderStage,
    constants: [*c]const CGPUConstantSpecialization,
    num_constants: u32,
};
pub const struct_CGPUSampler = extern struct {
    device: CGPUDeviceId,
};
pub const CGPUSamplerId = [*c]const struct_CGPUSampler;
pub const struct_CGPURootSignatureDescriptor = extern struct {
    shaders: [*c]struct_CGPUShaderEntryDescriptor,
    shader_count: u32,
    static_samplers: [*c]const CGPUSamplerId,
    static_sampler_names: [*c]const [*c]const char8_t,
    static_sampler_count: u32,
    push_constant_names: [*c]const [*c]const char8_t,
    push_constant_count: u32,
    pool: CGPURootSignaturePoolId,
};
pub const struct_CGPUParameterTable = extern struct {
    resources: [*c]CGPUShaderResource,
    resources_count: u32,
    set_index: u32,
};
pub const CGPUParameterTable = struct_CGPUParameterTable;
pub const struct_CGPURootSignature = extern struct {
    device: CGPUDeviceId,
    tables: [*c]CGPUParameterTable,
    table_count: u32,
    push_constants: [*c]CGPUShaderResource,
    push_constant_count: u32,
    static_samplers: [*c]CGPUShaderResource,
    static_sampler_count: u32,
    pipeline_type: ECGPUPipelineType,
    pool: CGPURootSignaturePoolId,
    pool_sig: CGPURootSignatureId,
};
pub const CGPURootSignatureId = [*c]const struct_CGPURootSignature;
pub const CGPUProcCreateRootSignature = ?fn (CGPUDeviceId, [*c]const struct_CGPURootSignatureDescriptor) callconv(.C) CGPURootSignatureId;
pub const CGPUProcFreeRootSignature = ?fn (CGPURootSignatureId) callconv(.C) void;
pub const struct_CGPUDescriptorSetDescriptor = extern struct {
    root_signature: CGPURootSignatureId,
    set_index: u32,
};
pub const struct_CGPUDescriptorSet = extern struct {
    root_signature: CGPURootSignatureId,
    index: u32,
};
pub const CGPUDescriptorSetId = [*c]const struct_CGPUDescriptorSet;
pub const CGPUProcCreateDescriptorSet = ?fn (CGPUDeviceId, [*c]const struct_CGPUDescriptorSetDescriptor) callconv(.C) CGPUDescriptorSetId;
pub const CGPUProcFreeDescriptorSet = ?fn (CGPUDescriptorSetId) callconv(.C) void;
const struct_unnamed_3 = extern struct {
    offsets: [*c]const u64,
    sizes: [*c]const u64,
};
const struct_unnamed_4 = extern struct {
    uav_mip_slice: u32,
    blend_mip_chain: bool,
};
const union_unnamed_2 = extern union {
    buffers_params: struct_unnamed_3,
    uav_params: struct_unnamed_4,
    enable_stencil_resource: bool,
}; // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu\api.h:1345:22: warning: struct demoted to opaque type - has bitfield
pub const struct_CGPUTexture = opaque {};
pub const CGPUTextureId = ?*const struct_CGPUTexture; // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu\api.h:1328:27: warning: struct demoted to opaque type - has bitfield
pub const struct_CGPUTextureViewDescriptor = opaque {};
pub const CGPUTextureViewDescriptor = struct_CGPUTextureViewDescriptor;
pub const struct_CGPUTextureView = extern struct {
    device: CGPUDeviceId,
    info: CGPUTextureViewDescriptor,
};
pub const CGPUTextureViewId = ?*const struct_CGPUTextureView; // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu\api.h:808:14: warning: struct demoted to opaque type - has bitfield
pub const struct_CGPUBuffer = opaque {};
pub const CGPUBufferId = ?*const struct_CGPUBuffer;
pub const struct_CGPURenderPipeline = extern struct {
    device: CGPUDeviceId,
    root_signature: CGPURootSignatureId,
};
pub const CGPURenderPipelineId = [*c]const struct_CGPURenderPipeline;
pub const struct_CGPUComputePipeline = extern struct {
    device: CGPUDeviceId,
    root_signature: CGPURootSignatureId,
};
pub const CGPUComputePipelineId = [*c]const struct_CGPUComputePipeline;
const union_unnamed_5 = extern union {
    ptrs: [*c]?*const anyopaque,
    textures: [*c]CGPUTextureViewId,
    samplers: [*c]CGPUSamplerId,
    buffers: [*c]CGPUBufferId,
    render_pipelines: [*c]CGPURenderPipelineId,
    compute_pipelines: [*c]CGPUComputePipelineId,
    descriptor_sets: [*c]CGPUDescriptorSetId,
};
pub const struct_CGPUDescriptorData = extern struct {
    name: [*c]const char8_t,
    binding: u32,
    binding_type: ECGPUResourceType,
    unnamed_0: union_unnamed_2,
    unnamed_1: union_unnamed_5,
    count: u32,
};
pub const CGPUProcUpdateDescriptorSet = ?fn (CGPUDescriptorSetId, [*c]const struct_CGPUDescriptorData, u32) callconv(.C) void;
pub const CGPUShaderEntryDescriptor = struct_CGPUShaderEntryDescriptor;
pub const struct_CGPUComputePipelineDescriptor = extern struct {
    root_signature: CGPURootSignatureId,
    compute_shader: [*c]CGPUShaderEntryDescriptor,
};
pub const CGPUProcCreateComputePipeline = ?fn (CGPUDeviceId, [*c]const struct_CGPUComputePipelineDescriptor) callconv(.C) CGPUComputePipelineId;
pub const CGPUProcFreeComputePipeline = ?fn (CGPUComputePipelineId) callconv(.C) void;
pub const struct_CGPUVertexAttribute = extern struct {
    semantic_name: [64]char8_t,
    array_size: u32,
    format: ECGPUFormat,
    binding: u32,
    offset: u32,
    elem_stride: u32,
    rate: ECGPUVertexInputRate,
};
pub const CGPUVertexAttribute = struct_CGPUVertexAttribute;
pub const struct_CGPUVertexLayout = extern struct {
    attribute_count: u32,
    attributes: [15]CGPUVertexAttribute,
};
pub const CGPUVertexLayout = struct_CGPUVertexLayout;
pub const struct_CGPUBlendStateDescriptor = extern struct {
    src_factors: [8]ECGPUBlendConstant,
    dst_factors: [8]ECGPUBlendConstant,
    src_alpha_factors: [8]ECGPUBlendConstant,
    dst_alpha_factors: [8]ECGPUBlendConstant,
    blend_modes: [8]ECGPUBlendMode,
    blend_alpha_modes: [8]ECGPUBlendMode,
    masks: [8]i32,
    alpha_to_coverage: bool,
    independent_blend: bool,
};
pub const CGPUBlendStateDescriptor = struct_CGPUBlendStateDescriptor;
pub const struct_CGPUDepthStateDesc = extern struct {
    depth_test: bool,
    depth_write: bool,
    depth_func: ECGPUCompareMode,
    stencil_test: bool,
    stencil_read_mask: u8,
    stencil_write_mask: u8,
    stencil_front_func: ECGPUCompareMode,
    stencil_front_fail: ECGPUStencilOp,
    depth_front_fail: ECGPUStencilOp,
    stencil_front_pass: ECGPUStencilOp,
    stencil_back_func: ECGPUCompareMode,
    stencil_back_fail: ECGPUStencilOp,
    depth_back_fail: ECGPUStencilOp,
    stencil_back_pass: ECGPUStencilOp,
};
pub const CGPUDepthStateDescriptor = struct_CGPUDepthStateDesc;
pub const struct_CGPURasterizerStateDescriptor = extern struct {
    cull_mode: ECGPUCullMode,
    depth_bias: i32,
    slope_scaled_depth_bias: f32,
    fill_mode: ECGPUFillMode,
    front_face: ECGPUFrontFace,
    enable_multi_sample: bool,
    enable_scissor: bool,
    enable_depth_clamp: bool,
};
pub const CGPURasterizerStateDescriptor = struct_CGPURasterizerStateDescriptor;
pub const struct_CGPURenderPipelineDescriptor = extern struct {
    root_signature: CGPURootSignatureId,
    vertex_shader: [*c]const CGPUShaderEntryDescriptor,
    tesc_shader: [*c]const CGPUShaderEntryDescriptor,
    tese_shader: [*c]const CGPUShaderEntryDescriptor,
    geom_shader: [*c]const CGPUShaderEntryDescriptor,
    fragment_shader: [*c]const CGPUShaderEntryDescriptor,
    vertex_layout: [*c]const CGPUVertexLayout,
    blend_state: [*c]const CGPUBlendStateDescriptor,
    depth_state: [*c]const CGPUDepthStateDescriptor,
    rasterizer_state: [*c]const CGPURasterizerStateDescriptor,
    color_formats: [*c]const ECGPUFormat,
    render_target_count: u32,
    sample_count: ECGPUSampleCount,
    sample_quality: u32,
    color_resolve_disable_mask: ECGPUSlotMask,
    depth_stencil_format: ECGPUFormat,
    prim_topology: ECGPUPrimitiveTopology,
    enable_indirect_command: bool,
};
pub const CGPUProcCreateRenderPipeline = ?fn (CGPUDeviceId, [*c]const struct_CGPURenderPipelineDescriptor) callconv(.C) CGPURenderPipelineId;
pub const CGPUProcFreeRenderPipeline = ?fn (CGPURenderPipelineId) callconv(.C) void;
pub const struct_CGPUMemoryPoolDescriptor = extern struct {
    mem_usage: ECGPUMemoryUsage,
    block_size: u64,
    min_block_count: u32,
    max_block_count: u32,
    min_alloc_alignment: u64,
};
pub const struct_CGPUMemoryPool = opaque {};
pub const CGPUMemoryPoolId = ?*const struct_CGPUMemoryPool;
pub const CGPUProcCreateMemoryPool = ?fn (CGPUDeviceId, [*c]const struct_CGPUMemoryPoolDescriptor) callconv(.C) CGPUMemoryPoolId;
pub const CGPUProcFreeMemoryPool = ?fn (CGPUMemoryPoolId) callconv(.C) void;
pub const struct_CGPUQueryPoolDescriptor = extern struct {
    type: ECGPUQueryType,
    query_count: u32,
};
pub const struct_CGPUQueryPool = extern struct {
    device: CGPUDeviceId,
    count: u32,
};
pub const CGPUQueryPoolId = [*c]const struct_CGPUQueryPool;
pub const CGPUProcCreateQueryPool = ?fn (CGPUDeviceId, [*c]const struct_CGPUQueryPoolDescriptor) callconv(.C) CGPUQueryPoolId;
pub const CGPUProcFreeQueryPool = ?fn (CGPUQueryPoolId) callconv(.C) void;
pub const struct_CGPUQueue = extern struct {
    device: CGPUDeviceId,
    type: ECGPUQueueType,
    index: CGPUQueueIndex,
};
pub const CGPUQueueId = [*c]const struct_CGPUQueue;
pub const CGPUProcGetQueue = ?fn (CGPUDeviceId, ECGPUQueueType, u32) callconv(.C) CGPUQueueId;
pub const struct_CGPUCommandPool = extern struct {
    queue: CGPUQueueId,
};
pub const CGPUCommandPoolId = [*c]const struct_CGPUCommandPool;
pub const struct_CGPUCommandBuffer = extern struct {
    device: CGPUDeviceId,
    pool: CGPUCommandPoolId,
    current_dispatch: ECGPUPipelineType,
};
pub const CGPUCommandBufferId = [*c]const struct_CGPUCommandBuffer;
pub const struct_CGPUQueueSubmitDescriptor = extern struct {
    cmds: [*c]CGPUCommandBufferId,
    signal_fence: CGPUFenceId,
    wait_semaphores: [*c]CGPUSemaphoreId,
    signal_semaphores: [*c]CGPUSemaphoreId,
    cmds_count: u32,
    wait_semaphore_count: u32,
    signal_semaphore_count: u32,
};
pub const CGPUProcSubmitQueue = ?fn (CGPUQueueId, [*c]const struct_CGPUQueueSubmitDescriptor) callconv(.C) void;
pub const CGPUProcWaitQueueIdle = ?fn (CGPUQueueId) callconv(.C) void;
pub const struct_CGPUSwapChain = extern struct {
    device: CGPUDeviceId,
    back_buffers: [*c]const CGPUTextureId,
    buffer_count: u32,
};
pub const CGPUSwapChainId = [*c]const struct_CGPUSwapChain;
pub const struct_CGPUQueuePresentDescriptor = extern struct {
    swapchain: CGPUSwapChainId,
    wait_semaphores: [*c]const CGPUSemaphoreId,
    wait_semaphore_count: u32,
    index: u8,
};
pub const CGPUProcQueuePresent = ?fn (CGPUQueueId, [*c]const struct_CGPUQueuePresentDescriptor) callconv(.C) void;
pub const CGPUProcQueueGetTimestampPeriodNS = ?fn (CGPUQueueId) callconv(.C) f32;
pub const CGPUProcFreeQueue = ?fn (CGPUQueueId) callconv(.C) void;
pub const struct_CGPUCommandPoolDescriptor = extern struct {
    ___nothing_and_useless__: u32,
};
pub const CGPUProcCreateCommandPool = ?fn (CGPUQueueId, [*c]const struct_CGPUCommandPoolDescriptor) callconv(.C) CGPUCommandPoolId; // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu\api.h:992:10: warning: struct demoted to opaque type - has bitfield
pub const struct_CGPUCommandBufferDescriptor = opaque {};
pub const CGPUProcCreateCommandBuffer = ?fn (CGPUCommandPoolId, ?*const struct_CGPUCommandBufferDescriptor) callconv(.C) CGPUCommandBufferId;
pub const CGPUProcResetCommandPool = ?fn (CGPUCommandPoolId) callconv(.C) void;
pub const CGPUProcFreeCommandBuffer = ?fn (CGPUCommandBufferId) callconv(.C) void;
pub const CGPUProcFreeCommandPool = ?fn (CGPUCommandPoolId) callconv(.C) void;
pub const struct_CGPUShaderLibraryDescriptor = extern struct {
    name: [*c]const char8_t,
    code: [*c]const u32,
    code_size: u32,
    stage: ECGPUShaderStage,
};
pub const CGPUProcCreateShaderLibrary = ?fn (CGPUDeviceId, [*c]const struct_CGPUShaderLibraryDescriptor) callconv(.C) CGPUShaderLibraryId;
pub const CGPUProcFreeShaderLibrary = ?fn (CGPUShaderLibraryId) callconv(.C) void;
pub const struct_Buffer = opaque {};
pub const struct_CGPUBufferDescriptor = extern struct {
    size: u64,
    count_buffer: ?*struct_Buffer,
    name: [*c]const char8_t,
    descriptors: CGPUResourceTypes,
    memory_usage: ECGPUMemoryUsage,
    format: ECGPUFormat,
    flags: CGPUBufferCreationFlags,
    first_element: u64,
    elemet_count: u64,
    element_stride: u64,
    owner_queue: CGPUQueueId,
    start_state: ECGPUResourceState,
    prefer_on_device: bool,
    prefer_on_host: bool,
};
pub const CGPUProcCreateBuffer = ?fn (CGPUDeviceId, [*c]const struct_CGPUBufferDescriptor) callconv(.C) CGPUBufferId;
pub const struct_CGPUBufferRange = extern struct {
    offset: u64,
    size: u64,
};
pub const CGPUProcMapBuffer = ?fn (CGPUBufferId, [*c]const struct_CGPUBufferRange) callconv(.C) void;
pub const CGPUProcUnmapBuffer = ?fn (CGPUBufferId) callconv(.C) void;
pub const CGPUProcFreeBuffer = ?fn (CGPUBufferId) callconv(.C) void;
pub const struct_CGPUSamplerDescriptor = extern struct {
    min_filter: ECGPUFilterType,
    mag_filter: ECGPUFilterType,
    mipmap_mode: ECGPUMipMapMode,
    address_u: ECGPUAddressMode,
    address_v: ECGPUAddressMode,
    address_w: ECGPUAddressMode,
    mip_lod_bias: f32,
    max_anisotropy: f32,
    compare_func: ECGPUCompareMode,
};
pub const CGPUProcCreateSampler = ?fn (CGPUDeviceId, [*c]const struct_CGPUSamplerDescriptor) callconv(.C) CGPUSamplerId;
pub const CGPUProcFreeSampler = ?fn (CGPUSamplerId) callconv(.C) void;
const struct_unnamed_6 = extern struct {
    r: f32,
    g: f32,
    b: f32,
    a: f32,
};
const struct_unnamed_7 = extern struct {
    depth: f32,
    stencil: u32,
};
pub const union_CGPUClearValue = extern union {
    unnamed_0: struct_unnamed_6,
    unnamed_1: struct_unnamed_7,
};
pub const CGPUClearValue = union_CGPUClearValue;
pub const struct_CGPUTextureDescriptor = extern struct {
    name: [*c]const char8_t,
    native_handle: ?*const anyopaque,
    flags: CGPUTextureCreationFlags,
    clear_value: CGPUClearValue,
    width: u32,
    height: u32,
    depth: u32,
    array_size: u32,
    format: ECGPUFormat,
    mip_levels: u32,
    sample_count: ECGPUSampleCount,
    sample_quality: u32,
    owner_queue: CGPUQueueId,
    start_state: ECGPUResourceState,
    descriptors: CGPUResourceTypes,
    is_dedicated: u32,
    is_aliasing: u32,
};
pub const CGPUProcCreateTexture = ?fn (CGPUDeviceId, [*c]const struct_CGPUTextureDescriptor) callconv(.C) CGPUTextureId;
pub const CGPUProcFreeTexture = ?fn (CGPUTextureId) callconv(.C) void;
pub const CGPUProcCreateTextureView = ?fn (CGPUDeviceId, ?*const struct_CGPUTextureViewDescriptor) callconv(.C) CGPUTextureViewId;
pub const CGPUProcFreeTextureView = ?fn (CGPUTextureViewId) callconv(.C) void;
pub const struct_CGPUTextureAliasingBindDescriptor = extern struct {
    aliased: CGPUTextureId,
    aliasing: CGPUTextureId,
};
pub const CGPUProcTryBindAliasingTexture = ?fn (CGPUDeviceId, [*c]const struct_CGPUTextureAliasingBindDescriptor) callconv(.C) bool;
pub const struct_CGPUExportTextureDescriptor = extern struct {
    texture: CGPUTextureId,
};
pub const CGPUProcExportSharedTextureHandle = ?fn (CGPUDeviceId, [*c]const struct_CGPUExportTextureDescriptor) callconv(.C) u64;
pub const struct_CGPUImportTextureDescriptor = extern struct {
    backend: ECGPUBackend,
    shared_handle: u64,
    width: u32,
    height: u32,
    depth: u32,
    size_in_bytes: u64,
    is_dedicated: u32,
    format: ECGPUFormat,
    mip_levels: u32,
};
pub const CGPUProcImportSharedTextureHandle = ?fn (CGPUDeviceId, [*c]const struct_CGPUImportTextureDescriptor) callconv(.C) CGPUTextureId;
pub const struct_CGPUSwapChainDescriptor = extern struct {
    present_queues: [*c]CGPUQueueId,
    present_queues_count: u32,
    surface: CGPUSurfaceId,
    image_count: u32,
    width: u32,
    height: u32,
    enable_vsync: bool,
    use_flip_swap_effect: bool,
    clear_value: [4]f32,
    format: ECGPUFormat,
};
pub const CGPUProcCreateSwapChain = ?fn (CGPUDeviceId, [*c]const struct_CGPUSwapChainDescriptor) callconv(.C) CGPUSwapChainId;
pub const struct_CGPUAcquireNextDescriptor = extern struct {
    signal_semaphore: CGPUSemaphoreId,
    fence: CGPUFenceId,
};
pub const CGPUProcAcquireNext = ?fn (CGPUSwapChainId, [*c]const struct_CGPUAcquireNextDescriptor) callconv(.C) u32;
pub const CGPUProcFreeSwapChain = ?fn (CGPUSwapChainId) callconv(.C) void;
pub const CGPUProcCmdBegin = ?fn (CGPUCommandBufferId) callconv(.C) void;
pub const struct_CGPUBufferToBufferTransfer = extern struct {
    dst: CGPUBufferId,
    dst_offset: u64,
    src: CGPUBufferId,
    src_offset: u64,
    size: u64,
};
pub const CGPUProcCmdTransferBufferToBuffer = ?fn (CGPUCommandBufferId, [*c]const struct_CGPUBufferToBufferTransfer) callconv(.C) void;
pub const struct_CGPUTextureSubresource = extern struct {
    aspects: CGPUTextureViewAspects,
    mip_level: u32,
    base_array_layer: u32,
    layer_count: u32,
};
pub const CGPUTextureSubresource = struct_CGPUTextureSubresource;
pub const struct_CGPUBufferToTextureTransfer = extern struct {
    dst: CGPUTextureId,
    dst_subresource: CGPUTextureSubresource,
    src: CGPUBufferId,
    src_offset: u64,
};
pub const CGPUProcCmdTransferBufferToTexture = ?fn (CGPUCommandBufferId, [*c]const struct_CGPUBufferToTextureTransfer) callconv(.C) void;
pub const struct_CGPUTextureToTextureTransfer = extern struct {
    src: CGPUTextureId,
    src_subresource: CGPUTextureSubresource,
    dst: CGPUTextureId,
    dst_subresource: CGPUTextureSubresource,
};
pub const CGPUProcCmdTransferTextureToTexture = ?fn (CGPUCommandBufferId, [*c]const struct_CGPUTextureToTextureTransfer) callconv(.C) void; // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu\api.h:944:13: warning: struct demoted to opaque type - has bitfield
pub const struct_CGPUBufferBarrier = opaque {};
pub const CGPUBufferBarrier = struct_CGPUBufferBarrier; // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu\api.h:957:13: warning: struct demoted to opaque type - has bitfield
pub const struct_CGPUTextureBarrier = opaque {};
pub const CGPUTextureBarrier = struct_CGPUTextureBarrier;
pub const struct_CGPUResourceBarrierDescriptor = extern struct {
    buffer_barriers: ?*const CGPUBufferBarrier,
    buffer_barriers_count: u32,
    texture_barriers: ?*const CGPUTextureBarrier,
    texture_barriers_count: u32,
};
pub const CGPUProcCmdResourceBarrier = ?fn (CGPUCommandBufferId, [*c]const struct_CGPUResourceBarrierDescriptor) callconv(.C) void;
pub const struct_CGPUQueryDescriptor = extern struct {
    index: u32,
    stage: ECGPUShaderStage,
};
pub const CGPUProcCmdBeginQuery = ?fn (CGPUCommandBufferId, CGPUQueryPoolId, [*c]const struct_CGPUQueryDescriptor) callconv(.C) void;
pub const CGPUProcCmdEndQuery = ?fn (CGPUCommandBufferId, CGPUQueryPoolId, [*c]const struct_CGPUQueryDescriptor) callconv(.C) void;
pub const CGPUProcCmdResetQueryPool = ?fn (CGPUCommandBufferId, CGPUQueryPoolId, u32, u32) callconv(.C) void;
pub const CGPUProcCmdResolveQuery = ?fn (CGPUCommandBufferId, CGPUQueryPoolId, CGPUBufferId, u32, u32) callconv(.C) void;
pub const CGPUProcCmdEnd = ?fn (CGPUCommandBufferId) callconv(.C) void;
pub const struct_CGPUComputePassDescriptor = extern struct {
    name: [*c]const char8_t,
};
pub const struct_CGPUComputePassEncoder = extern struct {
    device: CGPUDeviceId,
};
pub const CGPUComputePassEncoderId = [*c]const struct_CGPUComputePassEncoder;
pub const CGPUProcCmdBeginComputePass = ?fn (CGPUCommandBufferId, [*c]const struct_CGPUComputePassDescriptor) callconv(.C) CGPUComputePassEncoderId;
pub const CGPUProcComputeEncoderBindDescriptorSet = ?fn (CGPUComputePassEncoderId, CGPUDescriptorSetId) callconv(.C) void;
pub const CGPUProcComputeEncoderPushConstants = ?fn (CGPUComputePassEncoderId, CGPURootSignatureId, [*c]const char8_t, ?*const anyopaque) callconv(.C) void;
pub const CGPUProcComputeEncoderBindPipeline = ?fn (CGPUComputePassEncoderId, CGPUComputePipelineId) callconv(.C) void;
pub const CGPUProcComputeEncoderDispatch = ?fn (CGPUComputePassEncoderId, u32, u32, u32) callconv(.C) void;
pub const CGPUProcCmdEndComputePass = ?fn (CGPUCommandBufferId, CGPUComputePassEncoderId) callconv(.C) void;
pub const struct_CGPUColorAttachment = extern struct {
    view: CGPUTextureViewId,
    resolve_view: CGPUTextureViewId,
    load_action: ECGPULoadAction,
    store_action: ECGPUStoreAction,
    clear_color: CGPUClearValue,
};
pub const CGPUColorAttachment = struct_CGPUColorAttachment;
pub const struct_CGPUDepthStencilAttachment = extern struct {
    view: CGPUTextureViewId,
    depth_load_action: ECGPULoadAction,
    depth_store_action: ECGPUStoreAction,
    clear_depth: f32,
    write_depth: u8,
    stencil_load_action: ECGPULoadAction,
    stencil_store_action: ECGPUStoreAction,
    clear_stencil: u32,
    write_stencil: u8,
};
pub const CGPUDepthStencilAttachment = struct_CGPUDepthStencilAttachment;
pub const struct_CGPURenderPassDescriptor = extern struct {
    name: [*c]const char8_t,
    sample_count: ECGPUSampleCount,
    color_attachments: [*c]const CGPUColorAttachment,
    depth_stencil: [*c]const CGPUDepthStencilAttachment,
    render_target_count: u32,
};
pub const struct_CGPURenderPassEncoder = extern struct {
    device: CGPUDeviceId,
};
pub const CGPURenderPassEncoderId = [*c]const struct_CGPURenderPassEncoder;
pub const CGPUProcCmdBeginRenderPass = ?fn (CGPUCommandBufferId, [*c]const struct_CGPURenderPassDescriptor) callconv(.C) CGPURenderPassEncoderId;
pub const CGPUProcRenderEncoderSetShadingRate = ?fn (CGPURenderPassEncoderId, ECGPUShadingRate, ECGPUShadingRateCombiner, ECGPUShadingRateCombiner) callconv(.C) void;
pub const CGPUProcRenderEncoderBindDescriptorSet = ?fn (CGPURenderPassEncoderId, CGPUDescriptorSetId) callconv(.C) void;
pub const CGPUProcRenderEncoderBindPipeline = ?fn (CGPURenderPassEncoderId, CGPURenderPipelineId) callconv(.C) void;
pub const CGPUProcRendeEncoderBindVertexBuffers = ?fn (CGPURenderPassEncoderId, u32, [*c]const CGPUBufferId, [*c]const u32, [*c]const u32) callconv(.C) void;
pub const CGPUProcRendeEncoderBindIndexBuffer = ?fn (CGPURenderPassEncoderId, CGPUBufferId, u32, u64) callconv(.C) void;
pub const CGPUProcRenderEncoderPushConstants = ?fn (CGPURenderPassEncoderId, CGPURootSignatureId, [*c]const char8_t, ?*const anyopaque) callconv(.C) void;
pub const CGPUProcRenderEncoderSetViewport = ?fn (CGPURenderPassEncoderId, f32, f32, f32, f32, f32, f32) callconv(.C) void;
pub const CGPUProcRenderEncoderSetScissor = ?fn (CGPURenderPassEncoderId, u32, u32, u32, u32) callconv(.C) void;
pub const CGPUProcRenderEncoderDraw = ?fn (CGPURenderPassEncoderId, u32, u32) callconv(.C) void;
pub const CGPUProcRenderEncoderDrawInstanced = ?fn (CGPURenderPassEncoderId, u32, u32, u32, u32) callconv(.C) void;
pub const CGPUProcRenderEncoderDrawIndexed = ?fn (CGPURenderPassEncoderId, u32, u32, u32) callconv(.C) void;
pub const CGPUProcRenderEncoderDrawIndexedInstanced = ?fn (CGPURenderPassEncoderId, u32, u32, u32, u32, u32) callconv(.C) void;
pub const CGPUProcCmdEndRenderPass = ?fn (CGPUCommandBufferId, CGPURenderPassEncoderId) callconv(.C) void;
pub const struct_CGPUEventInfo = extern struct {
    name: [*c]const char8_t,
    color: [4]f32,
};
pub const CGPUEventInfo = struct_CGPUEventInfo;
pub const CGPUProcCmdBeginEvent = ?fn (CGPUCommandBufferId, [*c]const CGPUEventInfo) callconv(.C) void;
pub const struct_CGPUMarkerInfo = extern struct {
    name: [*c]const char8_t,
    color: [4]f32,
};
pub const CGPUMarkerInfo = struct_CGPUMarkerInfo;
pub const CGPUProcCmdSetMarker = ?fn (CGPUCommandBufferId, [*c]const CGPUMarkerInfo) callconv(.C) void;
pub const CGPUProcCmdEndEvent = ?fn (CGPUCommandBufferId) callconv(.C) void;
pub const CGPUProcQueryDStorageAvailability = ?fn (CGPUDeviceId) callconv(.C) ECGPUDStorageAvailability;
pub const struct_CGPUDStorageQueueDescriptor = extern struct {
    source: ECGPUDStorageSource,
    capacity: u16,
    priority: ECGPUDStoragePriority,
    name: [*c]const u8,
};
pub const struct_CGPUDStorageQueue = extern struct {
    device: CGPUDeviceId,
};
pub const CGPUDStorageQueueId = [*c]const struct_CGPUDStorageQueue;
pub const CGPUProcCreateDStorageQueue = ?fn (CGPUDeviceId, [*c]const struct_CGPUDStorageQueueDescriptor) callconv(.C) CGPUDStorageQueueId;
pub const struct_CGPUDStorageFile = opaque {};
pub const CGPUDStorageFileId = ?*const struct_CGPUDStorageFile;
pub const CGPUDStorageFileHandle = CGPUDStorageFileId;
pub const CGPUProcDStorageOpenFile = ?fn (CGPUDStorageQueueId, [*c]const u8) callconv(.C) CGPUDStorageFileHandle;
pub const struct_CGPUDStorageFileInfo = extern struct {
    file_size: u64,
};
pub const CGPUDStorageFileInfo = struct_CGPUDStorageFileInfo;
pub const CGPUProcDStorageQueryFileInfo = ?fn (CGPUDStorageQueueId, CGPUDStorageFileHandle, [*c]CGPUDStorageFileInfo) callconv(.C) void;
const struct_unnamed_8 = extern struct {
    bytes: [*c]u8,
    bytes_size: u64,
};
const struct_unnamed_9 = extern struct {
    file: CGPUDStorageFileHandle,
    offset: u64,
    size: u64,
};
pub const struct_CGPUDStorageBufferIODescriptor = extern struct {
    compression: CGPUDStorageCompression,
    source_type: ECGPUDStorageSource,
    source_memory: struct_unnamed_8,
    source_file: struct_unnamed_9,
    buffer: CGPUBufferId,
    offset: u64,
    uncompressed_size: u64,
    fence: CGPUFenceId,
    name: [*c]const u8,
};
pub const CGPUDStorageBufferIODescriptor = struct_CGPUDStorageBufferIODescriptor;
pub const CGPUProcDStorageEnqueueBufferRequest = ?fn (CGPUDStorageQueueId, [*c]const CGPUDStorageBufferIODescriptor) callconv(.C) void;
const struct_unnamed_10 = extern struct {
    bytes: [*c]const u8,
    bytes_size: u64,
};
const struct_unnamed_11 = extern struct {
    file: CGPUDStorageFileHandle,
    offset: u64,
    size: u64,
};
pub const struct_CGPUDStorageTextureIODescriptor = extern struct {
    compression: CGPUDStorageCompression,
    source_type: ECGPUDStorageSource,
    source_memory: struct_unnamed_10,
    source_file: struct_unnamed_11,
    texture: CGPUTextureId,
    width: u32,
    height: u32,
    depth: u32,
    uncompressed_size: u64,
    fence: CGPUFenceId,
    name: [*c]const u8,
};
pub const CGPUDStorageTextureIODescriptor = struct_CGPUDStorageTextureIODescriptor;
pub const CGPUProcDStorageEnqueueTextureRequest = ?fn (CGPUDStorageQueueId, [*c]const CGPUDStorageTextureIODescriptor) callconv(.C) void;
pub const CGPUProcDStorageQueueSubmit = ?fn (CGPUDStorageQueueId, CGPUFenceId) callconv(.C) void;
pub const CGPUProcDStorageCloseFile = ?fn (CGPUDStorageQueueId, CGPUDStorageFileHandle) callconv(.C) void;
pub const CGPUProcFreeDStorageQueue = ?fn (CGPUDStorageQueueId) callconv(.C) void;
pub const struct_CGPUProcTable = extern struct {

};
pub const CGPUProcTable = struct_CGPUProcTable;
pub const struct_HWND__ = opaque {};
pub const HWND = ?*struct_HWND__;
pub const CGPUSurfaceProc_CreateFromHWND = ?fn (CGPUDeviceId, HWND) callconv(.C) CGPUSurfaceId;
pub const CGPUSurfaceProc_Free = ?fn (CGPUDeviceId, CGPUSurfaceId) callconv(.C) void;
pub const struct_CGPUSurfacesProcTable = extern struct {

};
pub const CGPUSurfacesProcTable = struct_CGPUSurfacesProcTable;
pub const struct_CGPURuntimeTable = opaque {};
pub const struct_CGPUInstance = extern struct {
    proc_table: [*c]const CGPUProcTable,
    surfaces_table: [*c]const CGPUSurfacesProcTable,
    runtime_table: ?*struct_CGPURuntimeTable,
    backend: ECGPUBackend,
    nvapi_status: ECGPUNvAPI_Status,
    ags_status: ECGPUAGSReturnCode,
    enable_set_name: bool,
};
pub const struct_CGPUAdapterDescriptor = opaque {};
pub const struct_CGPUQueueDescriptor = opaque {};
pub const struct_CGPUSemaphoreDescriptor = opaque {};
pub const struct_CGPUFenceDescriptor = opaque {};
pub const struct_CGPURenderPassEncoderDescriptor = opaque {};
pub const struct_CGPUComputePassEncoderDescriptor = opaque {};
pub const struct_CGPUShaderReflectionDescriptor = opaque {};
pub const CGPUShaderReflectionId = [*c]const struct_CGPUShaderReflection;
pub const struct_CGPUPipelineReflectionDescriptor = opaque {};
pub const struct_CGPUPipelineReflection = extern struct {
    stages: [6][*c]CGPUShaderReflection,
    shader_resources: [*c]CGPUShaderResource,
    shader_resources_count: u32,
};
pub const CGPUPipelineReflectionId = [*c]const struct_CGPUPipelineReflection;
pub const struct_CGPUDStorageFileDescriptor = opaque {};
pub const CGPU_BUFFER_OUT_OF_HOST_MEMORY: CGPUBufferId = @intToPtr(CGPUBufferId, @as(c_int, 1));
pub const CGPU_BUFFER_OUT_OF_DEVICE_MEMORY: CGPUBufferId = @intToPtr(CGPUBufferId, @as(c_int, 3));
pub var gCGPUBackendNames: [5][*c]const char8_t = [5][*c]const char8_t{
    "vulkan",
    "d3d12",
    "d3d12(xbox)",
    "agc",
    "metal",
}; // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu\api.h:105:13: warning: struct demoted to opaque type - has bitfield
pub const struct_CGPUFormatSupport = opaque {};
pub const CGPUFormatSupport = struct_CGPUFormatSupport;
pub const CGPUInstanceFeatures = struct_CGPUInstanceFeatures;
pub const CGPUBufferRange = struct_CGPUBufferRange;
pub extern fn cgpu_instance_get_backend(instance: CGPUInstanceId) ECGPUBackend;
pub extern fn cgpu_create_instance(desc: [*c]const struct_CGPUInstanceDescriptor) CGPUInstanceId;
pub extern fn cgpu_query_instance_features(instance: CGPUInstanceId, features: [*c]struct_CGPUInstanceFeatures) void;
pub extern fn cgpu_free_instance(instance: CGPUInstanceId) void;
pub extern fn cgpu_enum_adapters(instance: CGPUInstanceId, adapters: [*c]CGPUAdapterId, adapters_num: [*c]u32) void;
pub extern fn cgpu_query_adapter_detail(adapter: CGPUAdapterId) ?*const struct_CGPUAdapterDetail;
pub extern fn cgpu_query_queue_count(adapter: CGPUAdapterId, @"type": ECGPUQueueType) u32;
pub extern fn cgpu_create_device(adapter: CGPUAdapterId, desc: [*c]const struct_CGPUDeviceDescriptor) CGPUDeviceId;
pub extern fn cgpu_query_video_memory_info(device: CGPUDeviceId, total: [*c]u64, used_bytes: [*c]u64) void;
pub extern fn cgpu_query_shared_memory_info(device: CGPUDeviceId, total: [*c]u64, used_bytes: [*c]u64) void;
pub extern fn cgpu_free_device(device: CGPUDeviceId) void;
pub extern fn cgpu_create_fence(device: CGPUDeviceId) CGPUFenceId;
pub extern fn cgpu_wait_fences(fences: [*c]const CGPUFenceId, fence_count: u32) void;
pub extern fn cgpu_query_fence_status(fence: CGPUFenceId) ECGPUFenceStatus;
pub extern fn cgpu_free_fence(fence: CGPUFenceId) void;
pub extern fn cgpu_create_semaphore(device: CGPUDeviceId) CGPUSemaphoreId;
pub extern fn cgpu_free_semaphore(semaphore: CGPUSemaphoreId) void;
pub extern fn cgpu_create_root_signature_pool(device: CGPUDeviceId, desc: [*c]const struct_CGPURootSignaturePoolDescriptor) CGPURootSignaturePoolId;
pub extern fn cgpu_free_root_signature_pool(pool: CGPURootSignaturePoolId) void;
pub extern fn cgpu_create_root_signature(device: CGPUDeviceId, desc: [*c]const struct_CGPURootSignatureDescriptor) CGPURootSignatureId;
pub extern fn cgpu_free_root_signature(signature: CGPURootSignatureId) void;
pub extern fn cgpu_create_descriptor_set(device: CGPUDeviceId, desc: [*c]const struct_CGPUDescriptorSetDescriptor) CGPUDescriptorSetId;
pub extern fn cgpu_update_descriptor_set(set: CGPUDescriptorSetId, datas: [*c]const struct_CGPUDescriptorData, count: u32) void;
pub extern fn cgpu_free_descriptor_set(set: CGPUDescriptorSetId) void;
pub extern fn cgpu_create_compute_pipeline(device: CGPUDeviceId, desc: [*c]const struct_CGPUComputePipelineDescriptor) CGPUComputePipelineId;
pub extern fn cgpu_free_compute_pipeline(pipeline: CGPUComputePipelineId) void;
pub extern fn cgpu_create_render_pipeline(device: CGPUDeviceId, desc: [*c]const struct_CGPURenderPipelineDescriptor) CGPURenderPipelineId;
pub extern fn cgpu_free_render_pipeline(pipeline: CGPURenderPipelineId) void;
pub extern fn cgpu_create_memory_pool(CGPUDeviceId, desc: [*c]const struct_CGPUMemoryPoolDescriptor) CGPUMemoryPoolId;
pub extern fn cgpu_free_memory_pool(pool: CGPUMemoryPoolId) void;
pub extern fn cgpu_create_query_pool(CGPUDeviceId, desc: [*c]const struct_CGPUQueryPoolDescriptor) CGPUQueryPoolId;
pub extern fn cgpu_free_query_pool(CGPUQueryPoolId) void;
pub extern fn cgpu_get_queue(device: CGPUDeviceId, @"type": ECGPUQueueType, index: u32) CGPUQueueId;
pub extern fn cgpu_submit_queue(queue: CGPUQueueId, desc: [*c]const struct_CGPUQueueSubmitDescriptor) void;
pub extern fn cgpu_queue_present(queue: CGPUQueueId, desc: [*c]const struct_CGPUQueuePresentDescriptor) void;
pub extern fn cgpu_wait_queue_idle(queue: CGPUQueueId) void;
pub extern fn cgpu_queue_get_timestamp_period_ns(queue: CGPUQueueId) f32;
pub extern fn cgpu_free_queue(queue: CGPUQueueId) void;
pub extern fn cgpu_create_command_pool(queue: CGPUQueueId, desc: [*c]const struct_CGPUCommandPoolDescriptor) CGPUCommandPoolId;
pub extern fn cgpu_create_command_buffer(pool: CGPUCommandPoolId, desc: ?*const struct_CGPUCommandBufferDescriptor) CGPUCommandBufferId;
pub extern fn cgpu_reset_command_pool(pool: CGPUCommandPoolId) void;
pub extern fn cgpu_free_command_buffer(cmd: CGPUCommandBufferId) void;
pub extern fn cgpu_free_command_pool(pool: CGPUCommandPoolId) void;
pub extern fn cgpu_create_shader_library(device: CGPUDeviceId, desc: [*c]const struct_CGPUShaderLibraryDescriptor) CGPUShaderLibraryId;
pub extern fn cgpu_free_shader_library(library: CGPUShaderLibraryId) void;
pub extern fn cgpu_create_buffer(device: CGPUDeviceId, desc: [*c]const struct_CGPUBufferDescriptor) CGPUBufferId;
pub extern fn cgpu_map_buffer(buffer: CGPUBufferId, range: [*c]const struct_CGPUBufferRange) void;
pub extern fn cgpu_unmap_buffer(buffer: CGPUBufferId) void;
pub extern fn cgpu_free_buffer(buffer: CGPUBufferId) void;
pub extern fn cgpu_create_sampler(device: CGPUDeviceId, desc: [*c]const struct_CGPUSamplerDescriptor) CGPUSamplerId;
pub extern fn cgpu_free_sampler(sampler: CGPUSamplerId) void;
pub extern fn cgpu_create_texture(device: CGPUDeviceId, desc: [*c]const struct_CGPUTextureDescriptor) CGPUTextureId;
pub extern fn cgpu_free_texture(texture: CGPUTextureId) void;
pub extern fn cgpu_create_texture_view(device: CGPUDeviceId, desc: ?*const struct_CGPUTextureViewDescriptor) CGPUTextureViewId;
pub extern fn cgpu_free_texture_view(render_target: CGPUTextureViewId) void;
pub extern fn cgpu_try_bind_aliasing_texture(device: CGPUDeviceId, desc: [*c]const struct_CGPUTextureAliasingBindDescriptor) bool;
pub extern fn cgpu_export_shared_texture_handle(device: CGPUDeviceId, desc: [*c]const struct_CGPUExportTextureDescriptor) u64;
pub extern fn cgpu_import_shared_texture_handle(device: CGPUDeviceId, desc: [*c]const struct_CGPUImportTextureDescriptor) CGPUTextureId;
pub extern fn cgpu_create_swapchain(device: CGPUDeviceId, desc: [*c]const struct_CGPUSwapChainDescriptor) CGPUSwapChainId;
pub extern fn cgpu_acquire_next_image(swapchain: CGPUSwapChainId, desc: [*c]const struct_CGPUAcquireNextDescriptor) u32;
pub extern fn cgpu_free_swapchain(swapchain: CGPUSwapChainId) void;
pub extern fn cgpu_cmd_begin(cmd: CGPUCommandBufferId) void;
pub extern fn cgpu_cmd_transfer_buffer_to_buffer(cmd: CGPUCommandBufferId, desc: [*c]const struct_CGPUBufferToBufferTransfer) void;
pub extern fn cgpu_cmd_transfer_texture_to_texture(cmd: CGPUCommandBufferId, desc: [*c]const struct_CGPUTextureToTextureTransfer) void;
pub extern fn cgpu_cmd_transfer_buffer_to_texture(cmd: CGPUCommandBufferId, desc: [*c]const struct_CGPUBufferToTextureTransfer) void;
pub extern fn cgpu_cmd_resource_barrier(cmd: CGPUCommandBufferId, desc: [*c]const struct_CGPUResourceBarrierDescriptor) void;
pub extern fn cgpu_cmd_begin_query(cmd: CGPUCommandBufferId, pool: CGPUQueryPoolId, desc: [*c]const struct_CGPUQueryDescriptor) void;
pub extern fn cgpu_cmd_end_query(cmd: CGPUCommandBufferId, pool: CGPUQueryPoolId, desc: [*c]const struct_CGPUQueryDescriptor) void;
pub extern fn cgpu_cmd_reset_query_pool(cmd: CGPUCommandBufferId, CGPUQueryPoolId, start_query: u32, query_count: u32) void;
pub extern fn cgpu_cmd_resolve_query(cmd: CGPUCommandBufferId, pool: CGPUQueryPoolId, readback: CGPUBufferId, start_query: u32, query_count: u32) void;
pub extern fn cgpu_cmd_end(cmd: CGPUCommandBufferId) void;
pub extern fn cgpu_cmd_begin_compute_pass(cmd: CGPUCommandBufferId, desc: [*c]const struct_CGPUComputePassDescriptor) CGPUComputePassEncoderId;
pub extern fn cgpu_compute_encoder_bind_descriptor_set(encoder: CGPUComputePassEncoderId, set: CGPUDescriptorSetId) void;
pub extern fn cgpu_compute_encoder_bind_pipeline(encoder: CGPUComputePassEncoderId, pipeline: CGPUComputePipelineId) void;
pub extern fn cgpu_compute_encoder_dispatch(encoder: CGPUComputePassEncoderId, X: u32, Y: u32, Z: u32) void;
pub extern fn cgpu_cmd_end_compute_pass(cmd: CGPUCommandBufferId, encoder: CGPUComputePassEncoderId) void;
pub extern fn cgpu_cmd_begin_render_pass(cmd: CGPUCommandBufferId, desc: [*c]const struct_CGPURenderPassDescriptor) CGPURenderPassEncoderId;
pub extern fn cgpu_render_encoder_set_shading_rate(encoder: CGPURenderPassEncoderId, shading_rate: ECGPUShadingRate, post_rasterizer_rate: ECGPUShadingRateCombiner, final_rate: ECGPUShadingRateCombiner) void;
pub extern fn cgpu_render_encoder_bind_descriptor_set(encoder: CGPURenderPassEncoderId, set: CGPUDescriptorSetId) void;
pub extern fn cgpu_render_encoder_set_viewport(encoder: CGPURenderPassEncoderId, x: f32, y: f32, width: f32, height: f32, min_depth: f32, max_depth: f32) void;
pub extern fn cgpu_render_encoder_set_scissor(encoder: CGPURenderPassEncoderId, x: u32, y: u32, width: u32, height: u32) void;
pub extern fn cgpu_render_encoder_bind_pipeline(encoder: CGPURenderPassEncoderId, pipeline: CGPURenderPipelineId) void;
pub extern fn cgpu_render_encoder_bind_vertex_buffers(encoder: CGPURenderPassEncoderId, buffer_count: u32, buffers: [*c]const CGPUBufferId, strides: [*c]const u32, offsets: [*c]const u32) void;
pub extern fn cgpu_render_encoder_bind_index_buffer(encoder: CGPURenderPassEncoderId, buffer: CGPUBufferId, index_stride: u32, offset: u64) void;
pub extern fn cgpu_render_encoder_push_constants(encoder: CGPURenderPassEncoderId, rs: CGPURootSignatureId, name: [*c]const char8_t, data: ?*const anyopaque) void;
pub extern fn cgpu_compute_encoder_push_constants(encoder: CGPUComputePassEncoderId, rs: CGPURootSignatureId, name: [*c]const char8_t, data: ?*const anyopaque) void;
pub extern fn cgpu_render_encoder_draw(encoder: CGPURenderPassEncoderId, vertex_count: u32, first_vertex: u32) void;
pub extern fn cgpu_render_encoder_draw_instanced(encoder: CGPURenderPassEncoderId, vertex_count: u32, first_vertex: u32, instance_count: u32, first_instance: u32) void;
pub extern fn cgpu_render_encoder_draw_indexed(encoder: CGPURenderPassEncoderId, index_count: u32, first_index: u32, first_vertex: u32) void;
pub extern fn cgpu_render_encoder_draw_indexed_instanced(encoder: CGPURenderPassEncoderId, index_count: u32, first_index: u32, instance_count: u32, first_instance: u32, first_vertex: u32) void;
pub extern fn cgpu_cmd_end_render_pass(cmd: CGPUCommandBufferId, encoder: CGPURenderPassEncoderId) void;
pub extern fn cgpu_cmd_begin_event(cmd: CGPUCommandBufferId, event: [*c]const CGPUEventInfo) void;
pub extern fn cgpu_cmd_set_marker(cmd: CGPUCommandBufferId, marker: [*c]const CGPUMarkerInfo) void;
pub extern fn cgpu_cmd_end_event(cmd: CGPUCommandBufferId) void;
pub extern fn cgpu_query_dstorage_availability(device: CGPUDeviceId) ECGPUDStorageAvailability;
pub extern fn cgpu_create_dstorage_queue(device: CGPUDeviceId, desc: [*c]const struct_CGPUDStorageQueueDescriptor) CGPUDStorageQueueId;
pub extern fn cgpu_dstorage_open_file(queue: CGPUDStorageQueueId, abs_path: [*c]const u8) CGPUDStorageFileHandle;
pub extern fn cgpu_dstorage_query_file_info(queue: CGPUDStorageQueueId, file: CGPUDStorageFileHandle, info: [*c]CGPUDStorageFileInfo) void;
pub extern fn cgpu_dstorage_enqueue_buffer_request(queue: CGPUDStorageQueueId, desc: [*c]const CGPUDStorageBufferIODescriptor) void;
pub extern fn cgpu_dstorage_enqueue_texture_request(queue: CGPUDStorageQueueId, desc: [*c]const CGPUDStorageTextureIODescriptor) void;
pub extern fn cgpu_dstorage_queue_submit(queue: CGPUDStorageQueueId, fence: CGPUFenceId) void;
pub extern fn cgpu_dstorage_close_file(queue: CGPUDStorageQueueId, file: CGPUDStorageFileHandle) void;
pub extern fn cgpu_free_dstorage_queue(queue: CGPUDStorageQueueId) void;
pub extern fn cgpux_create_mapped_constant_buffer(device: CGPUDeviceId, size: u64, name: [*c]const char8_t, device_local_preferred: bool) CGPUBufferId;
pub extern fn cgpux_create_mapped_upload_buffer(device: CGPUDeviceId, size: u64, name: [*c]const char8_t) CGPUBufferId;
pub extern fn cgpux_adapter_is_nvidia(adapter: CGPUAdapterId) bool;
pub extern fn cgpux_adapter_is_amd(adapter: CGPUAdapterId) bool;
pub extern fn cgpux_adapter_is_intel(adapter: CGPUAdapterId) bool;
pub extern fn cgpu_free_surface(device: CGPUDeviceId, surface: CGPUSurfaceId) void;
pub extern fn cgpu_surface_from_native_view(device: CGPUDeviceId, view: ?*anyopaque) CGPUSurfaceId;
pub extern fn cgpu_surface_from_hwnd(device: CGPUDeviceId, window: HWND) CGPUSurfaceId;
pub const struct_CGPUVendorPreset = extern struct {
    device_id: u32,
    vendor_id: u32,
    driver_version: u32,
    gpu_name: [64]u8,
};
pub const CGPUVendorPreset = struct_CGPUVendorPreset;
pub const CGPUAdapterDetail = struct_CGPUAdapterDetail;
pub const CGPUInstance = struct_CGPUInstance;
pub const CGPUAdapter = struct_CGPUAdapter;
pub const CGPUDevice = struct_CGPUDevice;
pub const CGPUQueue = struct_CGPUQueue;
pub const CGPUDStorageQueue = struct_CGPUDStorageQueue;
pub const CGPUFence = struct_CGPUFence;
pub const CGPUSemaphore = struct_CGPUSemaphore;
pub const CGPUCommandPool = struct_CGPUCommandPool;
pub const CGPUCommandBuffer = struct_CGPUCommandBuffer;
pub const CGPUQueryPool = struct_CGPUQueryPool;
pub const CGPUComputePassEncoder = struct_CGPUComputePassEncoder;
pub const CGPURenderPassEncoder = struct_CGPURenderPassEncoder;
pub const CGPUShaderLibrary = struct_CGPUShaderLibrary;
pub const CGPUPipelineReflection = struct_CGPUPipelineReflection;
pub const CGPUDescriptorData = struct_CGPUDescriptorData;
pub const CGPUBuffer = struct_CGPUBuffer;
pub const fastclear_0000: CGPUClearValue = CGPUClearValue{
    .unnamed_0 = struct_unnamed_6{
        .r = 0.0,
        .g = 0.0,
        .b = 0.0,
        .a = 0.0,
    },
};
pub const fastclear_0001: CGPUClearValue = CGPUClearValue{
    .unnamed_0 = struct_unnamed_6{
        .r = 0.0,
        .g = 0.0,
        .b = 0.0,
        .a = 1.0,
    },
};
pub const fastclear_1110: CGPUClearValue = CGPUClearValue{
    .unnamed_0 = struct_unnamed_6{
        .r = 1.0,
        .g = 1.0,
        .b = 1.0,
        .a = 1.0,
    },
};
pub const fastclear_1111: CGPUClearValue = CGPUClearValue{
    .unnamed_0 = struct_unnamed_6{
        .r = 1.0,
        .g = 1.0,
        .b = 1.0,
        .a = 1.0,
    },
};
pub const CGPUSwapChain = struct_CGPUSwapChain;
pub const CGPUInstanceDescriptor = struct_CGPUInstanceDescriptor;
pub const CGPUDStorageQueueDescriptor = struct_CGPUDStorageQueueDescriptor;
pub const CGPUQueueSubmitDescriptor = struct_CGPUQueueSubmitDescriptor;
pub const CGPUQueuePresentDescriptor = struct_CGPUQueuePresentDescriptor;
pub const CGPUQueryPoolDescriptor = struct_CGPUQueryPoolDescriptor;
pub const CGPUQueryDescriptor = struct_CGPUQueryDescriptor;
pub const CGPUAcquireNextDescriptor = struct_CGPUAcquireNextDescriptor;
pub const CGPUBufferToBufferTransfer = struct_CGPUBufferToBufferTransfer;
pub const CGPUTextureToTextureTransfer = struct_CGPUTextureToTextureTransfer;
pub const CGPUBufferToTextureTransfer = struct_CGPUBufferToTextureTransfer;
pub const CGPUResourceBarrierDescriptor = struct_CGPUResourceBarrierDescriptor;
pub const CGPUDeviceDescriptor = struct_CGPUDeviceDescriptor;
pub const CGPUCommandPoolDescriptor = struct_CGPUCommandPoolDescriptor;
pub const CGPUCommandBufferDescriptor = struct_CGPUCommandBufferDescriptor;
pub const CGPUSwapChainDescriptor = struct_CGPUSwapChainDescriptor;
pub const CGPUComputePassDescriptor = struct_CGPUComputePassDescriptor;
pub const CGPURenderPassDescriptor = struct_CGPURenderPassDescriptor;
pub const CGPURootSignaturePoolDescriptor = struct_CGPURootSignaturePoolDescriptor;
pub const CGPURootSignatureDescriptor = struct_CGPURootSignatureDescriptor;
pub const CGPUDescriptorSetDescriptor = struct_CGPUDescriptorSetDescriptor;
pub const CGPUComputePipelineDescriptor = struct_CGPUComputePipelineDescriptor;
pub const CGPURenderPipelineDescriptor = struct_CGPURenderPipelineDescriptor;
pub const CGPUMemoryPoolDescriptor = struct_CGPUMemoryPoolDescriptor;
pub const CGPURootSignaturePool = struct_CGPURootSignaturePool;
pub const CGPURootSignature = struct_CGPURootSignature;
pub const CGPUDescriptorSet = struct_CGPUDescriptorSet;
pub const CGPUComputePipeline = struct_CGPUComputePipeline;
pub const CGPURenderPipeline = struct_CGPURenderPipeline;
pub const CGPUShaderLibraryDescriptor = struct_CGPUShaderLibraryDescriptor;
pub const CGPUBufferDescriptor = struct_CGPUBufferDescriptor;
pub const CGPUTextureDescriptor = struct_CGPUTextureDescriptor;
pub const CGPUExportTextureDescriptor = struct_CGPUExportTextureDescriptor;
pub const CGPUImportTextureDescriptor = struct_CGPUImportTextureDescriptor;
pub const CGPUTextureAliasingBindDescriptor = struct_CGPUTextureAliasingBindDescriptor;
pub const CGPUTexture = struct_CGPUTexture;
pub const CGPUTextureView = struct_CGPUTextureView;
pub const CGPUSamplerDescriptor = struct_CGPUSamplerDescriptor;
pub const CGPUSampler = struct_CGPUSampler;
pub const __INTMAX_C_SUFFIX__ = @compileError("unable to translate macro: undefined identifier `LL`"); // (no file):79:9
pub const __UINTMAX_C_SUFFIX__ = @compileError("unable to translate macro: undefined identifier `ULL`"); // (no file):85:9
pub const __INT64_C_SUFFIX__ = @compileError("unable to translate macro: undefined identifier `LL`"); // (no file):169:9
pub const __UINT32_C_SUFFIX__ = @compileError("unable to translate macro: undefined identifier `U`"); // (no file):191:9
pub const __UINT64_C_SUFFIX__ = @compileError("unable to translate macro: undefined identifier `ULL`"); // (no file):199:9
pub const __seg_gs = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):328:9
pub const __seg_fs = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):329:9
pub const __declspec = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):400:9
pub const _cdecl = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):401:9
pub const __cdecl = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):402:9
pub const _stdcall = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):403:9
pub const __stdcall = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):404:9
pub const _fastcall = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):405:9
pub const __fastcall = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):406:9
pub const _thiscall = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):407:9
pub const __thiscall = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):408:9
pub const _pascal = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):409:9
pub const __pascal = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // (no file):410:9
pub const __stdint_join3 = @compileError("unable to translate C expr: unexpected token '##'"); // C:\zig-windows-x86_64-0.10.0-dev.3041+de62bd064\lib\include/stdint.h:245:9
pub const __int_c_join = @compileError("unable to translate C expr: unexpected token '##'"); // C:\zig-windows-x86_64-0.10.0-dev.3041+de62bd064\lib\include/stdint.h:282:9
pub const __uint_c = @compileError("unable to translate macro: undefined identifier `U`"); // C:\zig-windows-x86_64-0.10.0-dev.3041+de62bd064\lib\include/stdint.h:284:9
pub const __INTN_MIN = @compileError("unable to translate macro: undefined identifier `INT`"); // C:\zig-windows-x86_64-0.10.0-dev.3041+de62bd064\lib\include/stdint.h:776:10
pub const __INTN_MAX = @compileError("unable to translate macro: undefined identifier `INT`"); // C:\zig-windows-x86_64-0.10.0-dev.3041+de62bd064\lib\include/stdint.h:777:10
pub const __UINTN_MAX = @compileError("unable to translate macro: undefined identifier `UINT`"); // C:\zig-windows-x86_64-0.10.0-dev.3041+de62bd064\lib\include/stdint.h:778:9
pub const __INTN_C = @compileError("unable to translate macro: undefined identifier `INT`"); // C:\zig-windows-x86_64-0.10.0-dev.3041+de62bd064\lib\include/stdint.h:779:10
pub const __UINTN_C = @compileError("unable to translate macro: undefined identifier `UINT`"); // C:\zig-windows-x86_64-0.10.0-dev.3041+de62bd064\lib\include/stdint.h:780:9
pub const SKR_IF_CPP = @compileError("unable to translate C expr: expected ')' instead got '...'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:15:13
pub const SKR_ALIGNAS = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:21:13
pub const STRINGIFY_IMPL = @compileError("unable to translate C expr: expected ')' instead got '...'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:35:13
pub const STRINGIFY = @compileError("unable to translate C expr: expected ')' instead got '...'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:36:13
pub const sstatic_ctor_name_impl = @compileError("unable to translate C expr: unexpected token '#'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:39:9
pub const sstatic_ctor_name = @compileError("unable to translate C expr: unexpected token '#'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:40:9
pub const sattr = @compileError("unable to translate C expr: expected ')' instead got '...'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:54:13
pub const spush_attr = @compileError("unable to translate C expr: expected ')' instead got '...'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:55:13
pub const spop_attr = @compileError("unable to translate C expr: unexpected token 'Eof'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:56:13
pub const sstatic_ctor = @compileError("unable to translate C expr: unexpected token 'Eof'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:61:17
pub const sreflect_struct = @compileError("unable to translate C expr: expected ')' instead got '...'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:64:9
pub const sreflect_enum = @compileError("unable to translate C expr: expected ')' instead got '...'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:65:9
pub const sreflect_enum_class = @compileError("unable to translate C expr: expected ')' instead got '...'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:66:9
pub const simport_struct_impl_impl = @compileError("unable to translate macro: undefined identifier `import_`"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:67:9
pub const simport_struct = @compileError("unable to translate macro: undefined identifier `__COUNTER__`"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:69:9
pub const SKR_DISABLE_OPTIMIZATION = @compileError("unable to translate macro: undefined identifier `clang`"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:86:13
pub const SKR_ENABLE_OPTIMIZATION = @compileError("unable to translate macro: undefined identifier `clang`"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:87:13
pub const FORCEINLINE = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:114:13
pub const DEFINE_ALIGNED = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:116:13
pub const SKR_CONSTEXPR = @compileError("unable to translate C expr: unexpected token 'const'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:122:13
pub const SKRENUM = @compileError("unable to translate C expr: unexpected token 'Eof'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:134:13
pub const RUNTIME_IMPORT = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:141:17
pub const RUNTIME_EXPORT = @compileError("unable to translate macro: undefined identifier `__attribute__`"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:149:17
pub const DECLARE_ZERO = @compileError("unable to translate C expr: unexpected token '='"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:220:13
pub const DECLARE_ZERO_VLA = @compileError("unable to translate macro: undefined identifier `memset`"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:230:17
pub const SKR_TEMPLATE = @compileError("unable to translate macro: undefined identifier `template`"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:309:9
pub const RUNTIME_FORCEINLINE = @compileError("unable to translate C expr: unexpected token 'inline'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:354:17
pub const RUNTIME_INLINE = @compileError("unable to translate C expr: unexpected token 'inline'"); // c:\Coding\Sakura.Runtime\modules\runtime\include/platform/configure.h:357:9
pub const cgpu_static_assert = @compileError("unable to translate macro: undefined identifier `static_assert`"); // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu/cgpu_config.h:56:9
pub const cgpu_hash = @compileError("unable to translate macro: undefined identifier `skr_hash`"); // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu/cgpu_config.h:63:9
pub const CGPU_ARRAY_LEN = @compileError("unable to translate C expr: expected ')' instead got '['"); // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu\api.h:5:9
pub const DEFINE_CGPU_OBJECT = @compileError("unable to translate macro: undefined identifier `Descriptor`"); // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu\api.h:19:9
pub const CGPU_CHAINED_DESCRIPTOR_HEADER = @compileError("unable to translate macro: undefined identifier `backend`"); // c:\Coding\Sakura.Runtime\modules\runtime\include\cgpu\api.h:851:9
pub const __llvm__ = @as(c_int, 1);
pub const __clang__ = @as(c_int, 1);
pub const __clang_major__ = @as(c_int, 14);
pub const __clang_minor__ = @as(c_int, 0);
pub const __clang_patchlevel__ = @as(c_int, 6);
pub const __clang_version__ = "14.0.6 (git@github.com:ziglang/zig-bootstrap.git dbc902054739800b8c1656dc1fb29571bba074b9)";
pub const __GNUC__ = @as(c_int, 4);
pub const __GNUC_MINOR__ = @as(c_int, 2);
pub const __GNUC_PATCHLEVEL__ = @as(c_int, 1);
pub const __GXX_ABI_VERSION = @as(c_int, 1002);
pub const __ATOMIC_RELAXED = @as(c_int, 0);
pub const __ATOMIC_CONSUME = @as(c_int, 1);
pub const __ATOMIC_ACQUIRE = @as(c_int, 2);
pub const __ATOMIC_RELEASE = @as(c_int, 3);
pub const __ATOMIC_ACQ_REL = @as(c_int, 4);
pub const __ATOMIC_SEQ_CST = @as(c_int, 5);
pub const __OPENCL_MEMORY_SCOPE_WORK_ITEM = @as(c_int, 0);
pub const __OPENCL_MEMORY_SCOPE_WORK_GROUP = @as(c_int, 1);
pub const __OPENCL_MEMORY_SCOPE_DEVICE = @as(c_int, 2);
pub const __OPENCL_MEMORY_SCOPE_ALL_SVM_DEVICES = @as(c_int, 3);
pub const __OPENCL_MEMORY_SCOPE_SUB_GROUP = @as(c_int, 4);
pub const __PRAGMA_REDEFINE_EXTNAME = @as(c_int, 1);
pub const __VERSION__ = "Clang 14.0.6 (git@github.com:ziglang/zig-bootstrap.git dbc902054739800b8c1656dc1fb29571bba074b9)";
pub const __OBJC_BOOL_IS_BOOL = @as(c_int, 0);
pub const __CONSTANT_CFSTRINGS__ = @as(c_int, 1);
pub const __SEH__ = @as(c_int, 1);
pub const __clang_literal_encoding__ = "UTF-8";
pub const __clang_wide_literal_encoding__ = "UTF-16";
pub const __ORDER_LITTLE_ENDIAN__ = @as(c_int, 1234);
pub const __ORDER_BIG_ENDIAN__ = @as(c_int, 4321);
pub const __ORDER_PDP_ENDIAN__ = @as(c_int, 3412);
pub const __BYTE_ORDER__ = __ORDER_LITTLE_ENDIAN__;
pub const __LITTLE_ENDIAN__ = @as(c_int, 1);
pub const __CHAR_BIT__ = @as(c_int, 8);
pub const __BOOL_WIDTH__ = @as(c_int, 8);
pub const __SHRT_WIDTH__ = @as(c_int, 16);
pub const __INT_WIDTH__ = @as(c_int, 32);
pub const __LONG_WIDTH__ = @as(c_int, 32);
pub const __LLONG_WIDTH__ = @as(c_int, 64);
pub const __BITINT_MAXWIDTH__ = @as(c_int, 128);
pub const __SCHAR_MAX__ = @as(c_int, 127);
pub const __SHRT_MAX__ = @as(c_int, 32767);
pub const __INT_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_int, 2147483647, .decimal);
pub const __LONG_MAX__ = @as(c_long, 2147483647);
pub const __LONG_LONG_MAX__ = @as(c_longlong, 9223372036854775807);
pub const __WCHAR_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_int, 65535, .decimal);
pub const __WCHAR_WIDTH__ = @as(c_int, 16);
pub const __WINT_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_int, 65535, .decimal);
pub const __WINT_WIDTH__ = @as(c_int, 16);
pub const __INTMAX_MAX__ = @as(c_longlong, 9223372036854775807);
pub const __INTMAX_WIDTH__ = @as(c_int, 64);
pub const __SIZE_MAX__ = @as(c_ulonglong, 18446744073709551615);
pub const __SIZE_WIDTH__ = @as(c_int, 64);
pub const __UINTMAX_MAX__ = @as(c_ulonglong, 18446744073709551615);
pub const __UINTMAX_WIDTH__ = @as(c_int, 64);
pub const __PTRDIFF_MAX__ = @as(c_longlong, 9223372036854775807);
pub const __PTRDIFF_WIDTH__ = @as(c_int, 64);
pub const __INTPTR_MAX__ = @as(c_longlong, 9223372036854775807);
pub const __INTPTR_WIDTH__ = @as(c_int, 64);
pub const __UINTPTR_MAX__ = @as(c_ulonglong, 18446744073709551615);
pub const __UINTPTR_WIDTH__ = @as(c_int, 64);
pub const __SIZEOF_DOUBLE__ = @as(c_int, 8);
pub const __SIZEOF_FLOAT__ = @as(c_int, 4);
pub const __SIZEOF_INT__ = @as(c_int, 4);
pub const __SIZEOF_LONG__ = @as(c_int, 4);
pub const __SIZEOF_LONG_DOUBLE__ = @as(c_int, 16);
pub const __SIZEOF_LONG_LONG__ = @as(c_int, 8);
pub const __SIZEOF_POINTER__ = @as(c_int, 8);
pub const __SIZEOF_SHORT__ = @as(c_int, 2);
pub const __SIZEOF_PTRDIFF_T__ = @as(c_int, 8);
pub const __SIZEOF_SIZE_T__ = @as(c_int, 8);
pub const __SIZEOF_WCHAR_T__ = @as(c_int, 2);
pub const __SIZEOF_WINT_T__ = @as(c_int, 2);
pub const __SIZEOF_INT128__ = @as(c_int, 16);
pub const __INTMAX_TYPE__ = c_longlong;
pub const __INTMAX_FMTd__ = "lld";
pub const __INTMAX_FMTi__ = "lli";
pub const __UINTMAX_TYPE__ = c_ulonglong;
pub const __UINTMAX_FMTo__ = "llo";
pub const __UINTMAX_FMTu__ = "llu";
pub const __UINTMAX_FMTx__ = "llx";
pub const __UINTMAX_FMTX__ = "llX";
pub const __PTRDIFF_TYPE__ = c_longlong;
pub const __PTRDIFF_FMTd__ = "lld";
pub const __PTRDIFF_FMTi__ = "lli";
pub const __INTPTR_TYPE__ = c_longlong;
pub const __INTPTR_FMTd__ = "lld";
pub const __INTPTR_FMTi__ = "lli";
pub const __SIZE_TYPE__ = c_ulonglong;
pub const __SIZE_FMTo__ = "llo";
pub const __SIZE_FMTu__ = "llu";
pub const __SIZE_FMTx__ = "llx";
pub const __SIZE_FMTX__ = "llX";
pub const __WCHAR_TYPE__ = c_ushort;
pub const __WINT_TYPE__ = c_ushort;
pub const __SIG_ATOMIC_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_int, 2147483647, .decimal);
pub const __SIG_ATOMIC_WIDTH__ = @as(c_int, 32);
pub const __CHAR16_TYPE__ = c_ushort;
pub const __CHAR32_TYPE__ = c_uint;
pub const __UINTPTR_TYPE__ = c_ulonglong;
pub const __UINTPTR_FMTo__ = "llo";
pub const __UINTPTR_FMTu__ = "llu";
pub const __UINTPTR_FMTx__ = "llx";
pub const __UINTPTR_FMTX__ = "llX";
pub const __FLT_DENORM_MIN__ = @as(f32, 1.40129846e-45);
pub const __FLT_HAS_DENORM__ = @as(c_int, 1);
pub const __FLT_DIG__ = @as(c_int, 6);
pub const __FLT_DECIMAL_DIG__ = @as(c_int, 9);
pub const __FLT_EPSILON__ = @as(f32, 1.19209290e-7);
pub const __FLT_HAS_INFINITY__ = @as(c_int, 1);
pub const __FLT_HAS_QUIET_NAN__ = @as(c_int, 1);
pub const __FLT_MANT_DIG__ = @as(c_int, 24);
pub const __FLT_MAX_10_EXP__ = @as(c_int, 38);
pub const __FLT_MAX_EXP__ = @as(c_int, 128);
pub const __FLT_MAX__ = @as(f32, 3.40282347e+38);
pub const __FLT_MIN_10_EXP__ = -@as(c_int, 37);
pub const __FLT_MIN_EXP__ = -@as(c_int, 125);
pub const __FLT_MIN__ = @as(f32, 1.17549435e-38);
pub const __DBL_DENORM_MIN__ = 4.9406564584124654e-324;
pub const __DBL_HAS_DENORM__ = @as(c_int, 1);
pub const __DBL_DIG__ = @as(c_int, 15);
pub const __DBL_DECIMAL_DIG__ = @as(c_int, 17);
pub const __DBL_EPSILON__ = 2.2204460492503131e-16;
pub const __DBL_HAS_INFINITY__ = @as(c_int, 1);
pub const __DBL_HAS_QUIET_NAN__ = @as(c_int, 1);
pub const __DBL_MANT_DIG__ = @as(c_int, 53);
pub const __DBL_MAX_10_EXP__ = @as(c_int, 308);
pub const __DBL_MAX_EXP__ = @as(c_int, 1024);
pub const __DBL_MAX__ = 1.7976931348623157e+308;
pub const __DBL_MIN_10_EXP__ = -@as(c_int, 307);
pub const __DBL_MIN_EXP__ = -@as(c_int, 1021);
pub const __DBL_MIN__ = 2.2250738585072014e-308;
pub const __LDBL_DENORM_MIN__ = @as(c_longdouble, 3.64519953188247460253e-4951);
pub const __LDBL_HAS_DENORM__ = @as(c_int, 1);
pub const __LDBL_DIG__ = @as(c_int, 18);
pub const __LDBL_DECIMAL_DIG__ = @as(c_int, 21);
pub const __LDBL_EPSILON__ = @as(c_longdouble, 1.08420217248550443401e-19);
pub const __LDBL_HAS_INFINITY__ = @as(c_int, 1);
pub const __LDBL_HAS_QUIET_NAN__ = @as(c_int, 1);
pub const __LDBL_MANT_DIG__ = @as(c_int, 64);
pub const __LDBL_MAX_10_EXP__ = @as(c_int, 4932);
pub const __LDBL_MAX_EXP__ = @as(c_int, 16384);
pub const __LDBL_MAX__ = @as(c_longdouble, 1.18973149535723176502e+4932);
pub const __LDBL_MIN_10_EXP__ = -@as(c_int, 4931);
pub const __LDBL_MIN_EXP__ = -@as(c_int, 16381);
pub const __LDBL_MIN__ = @as(c_longdouble, 3.36210314311209350626e-4932);
pub const __POINTER_WIDTH__ = @as(c_int, 64);
pub const __BIGGEST_ALIGNMENT__ = @as(c_int, 16);
pub const __WCHAR_UNSIGNED__ = @as(c_int, 1);
pub const __WINT_UNSIGNED__ = @as(c_int, 1);
pub const __INT8_TYPE__ = i8;
pub const __INT8_FMTd__ = "hhd";
pub const __INT8_FMTi__ = "hhi";
pub const __INT8_C_SUFFIX__ = "";
pub const __INT16_TYPE__ = c_short;
pub const __INT16_FMTd__ = "hd";
pub const __INT16_FMTi__ = "hi";
pub const __INT16_C_SUFFIX__ = "";
pub const __INT32_TYPE__ = c_int;
pub const __INT32_FMTd__ = "d";
pub const __INT32_FMTi__ = "i";
pub const __INT32_C_SUFFIX__ = "";
pub const __INT64_TYPE__ = c_longlong;
pub const __INT64_FMTd__ = "lld";
pub const __INT64_FMTi__ = "lli";
pub const __UINT8_TYPE__ = u8;
pub const __UINT8_FMTo__ = "hho";
pub const __UINT8_FMTu__ = "hhu";
pub const __UINT8_FMTx__ = "hhx";
pub const __UINT8_FMTX__ = "hhX";
pub const __UINT8_C_SUFFIX__ = "";
pub const __UINT8_MAX__ = @as(c_int, 255);
pub const __INT8_MAX__ = @as(c_int, 127);
pub const __UINT16_TYPE__ = c_ushort;
pub const __UINT16_FMTo__ = "ho";
pub const __UINT16_FMTu__ = "hu";
pub const __UINT16_FMTx__ = "hx";
pub const __UINT16_FMTX__ = "hX";
pub const __UINT16_C_SUFFIX__ = "";
pub const __UINT16_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_int, 65535, .decimal);
pub const __INT16_MAX__ = @as(c_int, 32767);
pub const __UINT32_TYPE__ = c_uint;
pub const __UINT32_FMTo__ = "o";
pub const __UINT32_FMTu__ = "u";
pub const __UINT32_FMTx__ = "x";
pub const __UINT32_FMTX__ = "X";
pub const __UINT32_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_uint, 4294967295, .decimal);
pub const __INT32_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_int, 2147483647, .decimal);
pub const __UINT64_TYPE__ = c_ulonglong;
pub const __UINT64_FMTo__ = "llo";
pub const __UINT64_FMTu__ = "llu";
pub const __UINT64_FMTx__ = "llx";
pub const __UINT64_FMTX__ = "llX";
pub const __UINT64_MAX__ = @as(c_ulonglong, 18446744073709551615);
pub const __INT64_MAX__ = @as(c_longlong, 9223372036854775807);
pub const __INT_LEAST8_TYPE__ = i8;
pub const __INT_LEAST8_MAX__ = @as(c_int, 127);
pub const __INT_LEAST8_WIDTH__ = @as(c_int, 8);
pub const __INT_LEAST8_FMTd__ = "hhd";
pub const __INT_LEAST8_FMTi__ = "hhi";
pub const __UINT_LEAST8_TYPE__ = u8;
pub const __UINT_LEAST8_MAX__ = @as(c_int, 255);
pub const __UINT_LEAST8_FMTo__ = "hho";
pub const __UINT_LEAST8_FMTu__ = "hhu";
pub const __UINT_LEAST8_FMTx__ = "hhx";
pub const __UINT_LEAST8_FMTX__ = "hhX";
pub const __INT_LEAST16_TYPE__ = c_short;
pub const __INT_LEAST16_MAX__ = @as(c_int, 32767);
pub const __INT_LEAST16_WIDTH__ = @as(c_int, 16);
pub const __INT_LEAST16_FMTd__ = "hd";
pub const __INT_LEAST16_FMTi__ = "hi";
pub const __UINT_LEAST16_TYPE__ = c_ushort;
pub const __UINT_LEAST16_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_int, 65535, .decimal);
pub const __UINT_LEAST16_FMTo__ = "ho";
pub const __UINT_LEAST16_FMTu__ = "hu";
pub const __UINT_LEAST16_FMTx__ = "hx";
pub const __UINT_LEAST16_FMTX__ = "hX";
pub const __INT_LEAST32_TYPE__ = c_int;
pub const __INT_LEAST32_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_int, 2147483647, .decimal);
pub const __INT_LEAST32_WIDTH__ = @as(c_int, 32);
pub const __INT_LEAST32_FMTd__ = "d";
pub const __INT_LEAST32_FMTi__ = "i";
pub const __UINT_LEAST32_TYPE__ = c_uint;
pub const __UINT_LEAST32_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_uint, 4294967295, .decimal);
pub const __UINT_LEAST32_FMTo__ = "o";
pub const __UINT_LEAST32_FMTu__ = "u";
pub const __UINT_LEAST32_FMTx__ = "x";
pub const __UINT_LEAST32_FMTX__ = "X";
pub const __INT_LEAST64_TYPE__ = c_longlong;
pub const __INT_LEAST64_MAX__ = @as(c_longlong, 9223372036854775807);
pub const __INT_LEAST64_WIDTH__ = @as(c_int, 64);
pub const __INT_LEAST64_FMTd__ = "lld";
pub const __INT_LEAST64_FMTi__ = "lli";
pub const __UINT_LEAST64_TYPE__ = c_ulonglong;
pub const __UINT_LEAST64_MAX__ = @as(c_ulonglong, 18446744073709551615);
pub const __UINT_LEAST64_FMTo__ = "llo";
pub const __UINT_LEAST64_FMTu__ = "llu";
pub const __UINT_LEAST64_FMTx__ = "llx";
pub const __UINT_LEAST64_FMTX__ = "llX";
pub const __INT_FAST8_TYPE__ = i8;
pub const __INT_FAST8_MAX__ = @as(c_int, 127);
pub const __INT_FAST8_WIDTH__ = @as(c_int, 8);
pub const __INT_FAST8_FMTd__ = "hhd";
pub const __INT_FAST8_FMTi__ = "hhi";
pub const __UINT_FAST8_TYPE__ = u8;
pub const __UINT_FAST8_MAX__ = @as(c_int, 255);
pub const __UINT_FAST8_FMTo__ = "hho";
pub const __UINT_FAST8_FMTu__ = "hhu";
pub const __UINT_FAST8_FMTx__ = "hhx";
pub const __UINT_FAST8_FMTX__ = "hhX";
pub const __INT_FAST16_TYPE__ = c_short;
pub const __INT_FAST16_MAX__ = @as(c_int, 32767);
pub const __INT_FAST16_WIDTH__ = @as(c_int, 16);
pub const __INT_FAST16_FMTd__ = "hd";
pub const __INT_FAST16_FMTi__ = "hi";
pub const __UINT_FAST16_TYPE__ = c_ushort;
pub const __UINT_FAST16_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_int, 65535, .decimal);
pub const __UINT_FAST16_FMTo__ = "ho";
pub const __UINT_FAST16_FMTu__ = "hu";
pub const __UINT_FAST16_FMTx__ = "hx";
pub const __UINT_FAST16_FMTX__ = "hX";
pub const __INT_FAST32_TYPE__ = c_int;
pub const __INT_FAST32_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_int, 2147483647, .decimal);
pub const __INT_FAST32_WIDTH__ = @as(c_int, 32);
pub const __INT_FAST32_FMTd__ = "d";
pub const __INT_FAST32_FMTi__ = "i";
pub const __UINT_FAST32_TYPE__ = c_uint;
pub const __UINT_FAST32_MAX__ = @import("std").zig.c_translation.promoteIntLiteral(c_uint, 4294967295, .decimal);
pub const __UINT_FAST32_FMTo__ = "o";
pub const __UINT_FAST32_FMTu__ = "u";
pub const __UINT_FAST32_FMTx__ = "x";
pub const __UINT_FAST32_FMTX__ = "X";
pub const __INT_FAST64_TYPE__ = c_longlong;
pub const __INT_FAST64_MAX__ = @as(c_longlong, 9223372036854775807);
pub const __INT_FAST64_WIDTH__ = @as(c_int, 64);
pub const __INT_FAST64_FMTd__ = "lld";
pub const __INT_FAST64_FMTi__ = "lli";
pub const __UINT_FAST64_TYPE__ = c_ulonglong;
pub const __UINT_FAST64_MAX__ = @as(c_ulonglong, 18446744073709551615);
pub const __UINT_FAST64_FMTo__ = "llo";
pub const __UINT_FAST64_FMTu__ = "llu";
pub const __UINT_FAST64_FMTx__ = "llx";
pub const __UINT_FAST64_FMTX__ = "llX";
pub const __USER_LABEL_PREFIX__ = "";
pub const __FINITE_MATH_ONLY__ = @as(c_int, 0);
pub const __GNUC_STDC_INLINE__ = @as(c_int, 1);
pub const __GCC_ATOMIC_TEST_AND_SET_TRUEVAL = @as(c_int, 1);
pub const __CLANG_ATOMIC_BOOL_LOCK_FREE = @as(c_int, 2);
pub const __CLANG_ATOMIC_CHAR_LOCK_FREE = @as(c_int, 2);
pub const __CLANG_ATOMIC_CHAR16_T_LOCK_FREE = @as(c_int, 2);
pub const __CLANG_ATOMIC_CHAR32_T_LOCK_FREE = @as(c_int, 2);
pub const __CLANG_ATOMIC_WCHAR_T_LOCK_FREE = @as(c_int, 2);
pub const __CLANG_ATOMIC_SHORT_LOCK_FREE = @as(c_int, 2);
pub const __CLANG_ATOMIC_INT_LOCK_FREE = @as(c_int, 2);
pub const __CLANG_ATOMIC_LONG_LOCK_FREE = @as(c_int, 2);
pub const __CLANG_ATOMIC_LLONG_LOCK_FREE = @as(c_int, 2);
pub const __CLANG_ATOMIC_POINTER_LOCK_FREE = @as(c_int, 2);
pub const __GCC_ATOMIC_BOOL_LOCK_FREE = @as(c_int, 2);
pub const __GCC_ATOMIC_CHAR_LOCK_FREE = @as(c_int, 2);
pub const __GCC_ATOMIC_CHAR16_T_LOCK_FREE = @as(c_int, 2);
pub const __GCC_ATOMIC_CHAR32_T_LOCK_FREE = @as(c_int, 2);
pub const __GCC_ATOMIC_WCHAR_T_LOCK_FREE = @as(c_int, 2);
pub const __GCC_ATOMIC_SHORT_LOCK_FREE = @as(c_int, 2);
pub const __GCC_ATOMIC_INT_LOCK_FREE = @as(c_int, 2);
pub const __GCC_ATOMIC_LONG_LOCK_FREE = @as(c_int, 2);
pub const __GCC_ATOMIC_LLONG_LOCK_FREE = @as(c_int, 2);
pub const __GCC_ATOMIC_POINTER_LOCK_FREE = @as(c_int, 2);
pub const __NO_INLINE__ = @as(c_int, 1);
pub const __PIC__ = @as(c_int, 2);
pub const __pic__ = @as(c_int, 2);
pub const __FLT_EVAL_METHOD__ = @as(c_int, 0);
pub const __FLT_RADIX__ = @as(c_int, 2);
pub const __DECIMAL_DIG__ = __LDBL_DECIMAL_DIG__;
pub const __GCC_ASM_FLAG_OUTPUTS__ = @as(c_int, 1);
pub const __code_model_small__ = @as(c_int, 1);
pub const __amd64__ = @as(c_int, 1);
pub const __amd64 = @as(c_int, 1);
pub const __x86_64 = @as(c_int, 1);
pub const __x86_64__ = @as(c_int, 1);
pub const __SEG_GS = @as(c_int, 1);
pub const __SEG_FS = @as(c_int, 1);
pub const __k8 = @as(c_int, 1);
pub const __k8__ = @as(c_int, 1);
pub const __tune_k8__ = @as(c_int, 1);
pub const __REGISTER_PREFIX__ = "";
pub const __NO_MATH_INLINES = @as(c_int, 1);
pub const __AES__ = @as(c_int, 1);
pub const __VAES__ = @as(c_int, 1);
pub const __PCLMUL__ = @as(c_int, 1);
pub const __VPCLMULQDQ__ = @as(c_int, 1);
pub const __LAHF_SAHF__ = @as(c_int, 1);
pub const __LZCNT__ = @as(c_int, 1);
pub const __RDRND__ = @as(c_int, 1);
pub const __FSGSBASE__ = @as(c_int, 1);
pub const __BMI__ = @as(c_int, 1);
pub const __BMI2__ = @as(c_int, 1);
pub const __POPCNT__ = @as(c_int, 1);
pub const __PRFCHW__ = @as(c_int, 1);
pub const __RDSEED__ = @as(c_int, 1);
pub const __ADX__ = @as(c_int, 1);
pub const __MOVBE__ = @as(c_int, 1);
pub const __FMA__ = @as(c_int, 1);
pub const __F16C__ = @as(c_int, 1);
pub const __GFNI__ = @as(c_int, 1);
pub const __SHA__ = @as(c_int, 1);
pub const __FXSR__ = @as(c_int, 1);
pub const __XSAVE__ = @as(c_int, 1);
pub const __XSAVEOPT__ = @as(c_int, 1);
pub const __XSAVEC__ = @as(c_int, 1);
pub const __XSAVES__ = @as(c_int, 1);
pub const __CLFLUSHOPT__ = @as(c_int, 1);
pub const __CLWB__ = @as(c_int, 1);
pub const __SHSTK__ = @as(c_int, 1);
pub const __RDPID__ = @as(c_int, 1);
pub const __WAITPKG__ = @as(c_int, 1);
pub const __MOVDIRI__ = @as(c_int, 1);
pub const __MOVDIR64B__ = @as(c_int, 1);
pub const __PCONFIG__ = @as(c_int, 1);
pub const __PTWRITE__ = @as(c_int, 1);
pub const __INVPCID__ = @as(c_int, 1);
pub const __AVX2__ = @as(c_int, 1);
pub const __AVX__ = @as(c_int, 1);
pub const __SSE4_2__ = @as(c_int, 1);
pub const __SSE4_1__ = @as(c_int, 1);
pub const __SSSE3__ = @as(c_int, 1);
pub const __SSE3__ = @as(c_int, 1);
pub const __SSE2__ = @as(c_int, 1);
pub const __SSE2_MATH__ = @as(c_int, 1);
pub const __SSE__ = @as(c_int, 1);
pub const __SSE_MATH__ = @as(c_int, 1);
pub const __MMX__ = @as(c_int, 1);
pub const __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 = @as(c_int, 1);
pub const __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 = @as(c_int, 1);
pub const __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 = @as(c_int, 1);
pub const __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 = @as(c_int, 1);
pub const __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16 = @as(c_int, 1);
pub const __SIZEOF_FLOAT128__ = @as(c_int, 16);
pub const _WIN32 = @as(c_int, 1);
pub const _WIN64 = @as(c_int, 1);
pub const WIN32 = @as(c_int, 1);
pub const __WIN32 = @as(c_int, 1);
pub const __WIN32__ = @as(c_int, 1);
pub const WINNT = @as(c_int, 1);
pub const __WINNT = @as(c_int, 1);
pub const __WINNT__ = @as(c_int, 1);
pub const WIN64 = @as(c_int, 1);
pub const __WIN64 = @as(c_int, 1);
pub const __WIN64__ = @as(c_int, 1);
pub const __MINGW64__ = @as(c_int, 1);
pub const __MSVCRT__ = @as(c_int, 1);
pub const __MINGW32__ = @as(c_int, 1);
pub const __STDC__ = @as(c_int, 1);
pub const __STDC_HOSTED__ = @as(c_int, 1);
pub const __STDC_VERSION__ = @as(c_long, 201710);
pub const __STDC_UTF_16__ = @as(c_int, 1);
pub const __STDC_UTF_32__ = @as(c_int, 1);
pub const _DEBUG = @as(c_int, 1);
pub const __STDBOOL_H = "";
pub const @"bool" = bool;
pub const @"true" = @as(c_int, 1);
pub const @"false" = @as(c_int, 0);
pub const __bool_true_false_are_defined = @as(c_int, 1);
pub const __CLANG_STDINT_H = "";
pub const __int_least64_t = i64;
pub const __uint_least64_t = u64;
pub const __int_least32_t = i64;
pub const __uint_least32_t = u64;
pub const __int_least16_t = i64;
pub const __uint_least16_t = u64;
pub const __int_least8_t = i64;
pub const __uint_least8_t = u64;
pub const __uint32_t_defined = "";
pub const __int8_t_defined = "";
pub const __intptr_t_defined = "";
pub const _INTPTR_T = "";
pub const _UINTPTR_T = "";
pub inline fn __int_c(v: anytype, suffix: anytype) @TypeOf(__int_c_join(v, suffix)) {
    return __int_c_join(v, suffix);
}
pub const __int64_c_suffix = __INT64_C_SUFFIX__;
pub const __int32_c_suffix = __INT64_C_SUFFIX__;
pub const __int16_c_suffix = __INT64_C_SUFFIX__;
pub const __int8_c_suffix = __INT64_C_SUFFIX__;
pub inline fn INT64_C(v: anytype) @TypeOf(__int_c(v, __int64_c_suffix)) {
    return __int_c(v, __int64_c_suffix);
}
pub inline fn UINT64_C(v: anytype) @TypeOf(__uint_c(v, __int64_c_suffix)) {
    return __uint_c(v, __int64_c_suffix);
}
pub inline fn INT32_C(v: anytype) @TypeOf(__int_c(v, __int32_c_suffix)) {
    return __int_c(v, __int32_c_suffix);
}
pub inline fn UINT32_C(v: anytype) @TypeOf(__uint_c(v, __int32_c_suffix)) {
    return __uint_c(v, __int32_c_suffix);
}
pub inline fn INT16_C(v: anytype) @TypeOf(__int_c(v, __int16_c_suffix)) {
    return __int_c(v, __int16_c_suffix);
}
pub inline fn UINT16_C(v: anytype) @TypeOf(__uint_c(v, __int16_c_suffix)) {
    return __uint_c(v, __int16_c_suffix);
}
pub inline fn INT8_C(v: anytype) @TypeOf(__int_c(v, __int8_c_suffix)) {
    return __int_c(v, __int8_c_suffix);
}
pub inline fn UINT8_C(v: anytype) @TypeOf(__uint_c(v, __int8_c_suffix)) {
    return __uint_c(v, __int8_c_suffix);
}
pub const INT64_MAX = INT64_C(@import("std").zig.c_translation.promoteIntLiteral(c_int, 9223372036854775807, .decimal));
pub const INT64_MIN = -INT64_C(@import("std").zig.c_translation.promoteIntLiteral(c_int, 9223372036854775807, .decimal)) - @as(c_int, 1);
pub const UINT64_MAX = UINT64_C(@import("std").zig.c_translation.promoteIntLiteral(c_int, 18446744073709551615, .decimal));
pub const __INT_LEAST64_MIN = INT64_MIN;
pub const __INT_LEAST64_MAX = INT64_MAX;
pub const __UINT_LEAST64_MAX = UINT64_MAX;
pub const __INT_LEAST32_MIN = INT64_MIN;
pub const __INT_LEAST32_MAX = INT64_MAX;
pub const __UINT_LEAST32_MAX = UINT64_MAX;
pub const __INT_LEAST16_MIN = INT64_MIN;
pub const __INT_LEAST16_MAX = INT64_MAX;
pub const __UINT_LEAST16_MAX = UINT64_MAX;
pub const __INT_LEAST8_MIN = INT64_MIN;
pub const __INT_LEAST8_MAX = INT64_MAX;
pub const __UINT_LEAST8_MAX = UINT64_MAX;
pub const INT_LEAST64_MIN = __INT_LEAST64_MIN;
pub const INT_LEAST64_MAX = __INT_LEAST64_MAX;
pub const UINT_LEAST64_MAX = __UINT_LEAST64_MAX;
pub const INT_FAST64_MIN = __INT_LEAST64_MIN;
pub const INT_FAST64_MAX = __INT_LEAST64_MAX;
pub const UINT_FAST64_MAX = __UINT_LEAST64_MAX;
pub const INT32_MAX = INT32_C(@import("std").zig.c_translation.promoteIntLiteral(c_int, 2147483647, .decimal));
pub const INT32_MIN = -INT32_C(@import("std").zig.c_translation.promoteIntLiteral(c_int, 2147483647, .decimal)) - @as(c_int, 1);
pub const UINT32_MAX = UINT32_C(@import("std").zig.c_translation.promoteIntLiteral(c_int, 4294967295, .decimal));
pub const INT_LEAST32_MIN = __INT_LEAST32_MIN;
pub const INT_LEAST32_MAX = __INT_LEAST32_MAX;
pub const UINT_LEAST32_MAX = __UINT_LEAST32_MAX;
pub const INT_FAST32_MIN = __INT_LEAST32_MIN;
pub const INT_FAST32_MAX = __INT_LEAST32_MAX;
pub const UINT_FAST32_MAX = __UINT_LEAST32_MAX;
pub const INT16_MAX = INT16_C(@as(c_int, 32767));
pub const INT16_MIN = -INT16_C(@as(c_int, 32767)) - @as(c_int, 1);
pub const UINT16_MAX = UINT16_C(@import("std").zig.c_translation.promoteIntLiteral(c_int, 65535, .decimal));
pub const INT_LEAST16_MIN = __INT_LEAST16_MIN;
pub const INT_LEAST16_MAX = __INT_LEAST16_MAX;
pub const UINT_LEAST16_MAX = __UINT_LEAST16_MAX;
pub const INT_FAST16_MIN = __INT_LEAST16_MIN;
pub const INT_FAST16_MAX = __INT_LEAST16_MAX;
pub const UINT_FAST16_MAX = __UINT_LEAST16_MAX;
pub const INT8_MAX = INT8_C(@as(c_int, 127));
pub const INT8_MIN = -INT8_C(@as(c_int, 127)) - @as(c_int, 1);
pub const UINT8_MAX = UINT8_C(@as(c_int, 255));
pub const INT_LEAST8_MIN = __INT_LEAST8_MIN;
pub const INT_LEAST8_MAX = __INT_LEAST8_MAX;
pub const UINT_LEAST8_MAX = __UINT_LEAST8_MAX;
pub const INT_FAST8_MIN = __INT_LEAST8_MIN;
pub const INT_FAST8_MAX = __INT_LEAST8_MAX;
pub const UINT_FAST8_MAX = __UINT_LEAST8_MAX;
pub const INTPTR_MIN = -__INTPTR_MAX__ - @as(c_int, 1);
pub const INTPTR_MAX = __INTPTR_MAX__;
pub const UINTPTR_MAX = __UINTPTR_MAX__;
pub const PTRDIFF_MIN = -__PTRDIFF_MAX__ - @as(c_int, 1);
pub const PTRDIFF_MAX = __PTRDIFF_MAX__;
pub const SIZE_MAX = __SIZE_MAX__;
pub const INTMAX_MIN = -__INTMAX_MAX__ - @as(c_int, 1);
pub const INTMAX_MAX = __INTMAX_MAX__;
pub const UINTMAX_MAX = __UINTMAX_MAX__;
pub const SIG_ATOMIC_MIN = __INTN_MIN(__SIG_ATOMIC_WIDTH__);
pub const SIG_ATOMIC_MAX = __INTN_MAX(__SIG_ATOMIC_WIDTH__);
pub const WINT_MIN = __UINTN_C(__WINT_WIDTH__, @as(c_int, 0));
pub const WINT_MAX = __UINTN_MAX(__WINT_WIDTH__);
pub const WCHAR_MAX = __WCHAR_MAX__;
pub const WCHAR_MIN = __UINTN_C(__WCHAR_WIDTH__, @as(c_int, 0));
pub inline fn INTMAX_C(v: anytype) @TypeOf(__int_c(v, __INTMAX_C_SUFFIX__)) {
    return __int_c(v, __INTMAX_C_SUFFIX__);
}
pub inline fn UINTMAX_C(v: anytype) @TypeOf(__int_c(v, __UINTMAX_C_SUFFIX__)) {
    return __int_c(v, __UINTMAX_C_SUFFIX__);
}
pub const SKR_IS_BIG_ENDIAN = @as(c_int, 0);
pub const SKR_IS_LITTLE_ENDIAN = @as(c_int, 1);
pub inline fn SKR_ASSUME(x: anytype) @TypeOf(__builtin_assume(x)) {
    return __builtin_assume(x);
}
pub const sreflect = "";
pub const sfull_reflect = "";
pub const snoreflect = "";
pub inline fn simport_struct_impl(idx: anytype, name: anytype) @TypeOf(simport_struct_impl_impl(idx, name)) {
    return simport_struct_impl_impl(idx, name);
}
pub const SKR_OS_WINDOWS = "";
pub const RUNTIME_EXTERN_C = "";
pub const SKR_PLATFORM_X86_64 = "";
pub const RUNTIME_API = "";
pub const RUNTIME_LOCAL = "";
pub const SKR_IMPORT_API = RUNTIME_EXTERN_C ++ RUNTIME_IMPORT;
pub const CHAR8_T_DEFINED = "";
pub const PTR_SIZE = @as(c_int, 8);
pub const SKR_PLATFORM_AVX = "";
pub const SKR_PLATFORM_AVX2 = "";
pub const SKR_PLATFORM_64BIT = "";
pub const SKR_PLATFORM_LITTLE_ENDIAN = "";
pub const SKR_PLATFORM_SSE = "";
pub const SKR_PLATFORM_SSE2 = "";
pub const RUNTIME_COMPILER_CLANG = "";
pub const RUNTIME_COMPILER_VERSION = (__clang_major__ * @as(c_int, 100)) + __clang_minor__;
pub const RUNTIME_NOVTABLE = "";
pub const SKR_NOEXCEPT = "";
pub const SKR_HEADER_SCOPE_DEFINING_PLATFORM_CONFIGURE = "";
pub const SKR_WINDOWS_CONFIGURE_H = "";
pub const SKR_RUNTIME_USE_MIMALLOC = "";
pub const OS_DPI = @as(c_int, 96);
pub const KINDA_SMALL_NUMBER = 1.0e-4;
pub const SMALL_NUMBER = 1.0e-8;
pub const THRESH_VECTOR_NORMALIZED = 0.01;
pub const USE_DXMATH = "";
pub const SKR_RESOURCE_DEV_MODE = "";
pub const TRACY_ENABLE = "";
pub const TRACY_IMPORTS = "";
pub const TRACY_ON_DEMAND = "";
pub const TRACY_FIBERS = "";
pub const TRACY_TRACE_ALLOCATION = "";
pub const CGPU_USE_VULKAN = "";
pub const CGPU_USE_D3D12 = "";
pub const CGPU_EXTERN_C = "";
pub const CGPU_NULL = @as(c_int, 0);
pub const CGPU_NULLPTR = CGPU_NULL;
pub const MAX_GPU_VENDOR_STRING_LENGTH = @as(c_int, 64);
pub const MAX_GPU_DEBUG_NAME_LENGTH = @as(c_int, 128);
pub const PSO_NAME_LENGTH = @as(c_int, 160);
pub inline fn cgpu_max(a: anytype, b: anytype) @TypeOf(if (a > b) a else b) {
    return if (a > b) a else b;
}
pub inline fn cgpu_min(a: anytype, b: anytype) @TypeOf(if (a < b) a else b) {
    return if (a < b) a else b;
}
pub const CGPU_THREAD_SAFETY = "";
pub const CGPU_NAME_HASH_SEED = @import("std").zig.c_translation.promoteIntLiteral(c_int, 8053064571610612741, .decimal);
pub inline fn cgpu_name_hash(buffer: anytype, size: anytype) @TypeOf(cgpu_hash(buffer, size, CGPU_NAME_HASH_SEED)) {
    return cgpu_hash(buffer, size, CGPU_NAME_HASH_SEED);
}
pub const ENABLE_NSIGHT_AFTERMATH = "";
pub const CGPU_MAX_MRT_COUNT = @as(c_uint, 8);
pub const CGPU_MAX_VERTEX_ATTRIBS = @as(c_int, 15);
pub const CGPU_MAX_VERTEX_BINDINGS = @as(c_int, 15);
pub const CGPU_COLOR_MASK_RED = @as(c_int, 0x1);
pub const CGPU_COLOR_MASK_GREEN = @as(c_int, 0x2);
pub const CGPU_COLOR_MASK_BLUE = @as(c_int, 0x4);
pub const CGPU_COLOR_MASK_ALPHA = @as(c_int, 0x8);
pub const CGPU_COLOR_MASK_ALL = ((CGPU_COLOR_MASK_RED | CGPU_COLOR_MASK_GREEN) | CGPU_COLOR_MASK_BLUE) | CGPU_COLOR_MASK_ALPHA;
pub const CGPU_COLOR_MASK_NONE = @as(c_int, 0);
pub const CGPU_DSTORAGE_MAX_QUEUE_CAPACITY = @as(c_int, 0x2000);
pub const CGPU_SINGLE_GPU_NODE_COUNT = @as(c_int, 1);
pub const CGPU_SINGLE_GPU_NODE_MASK = @as(c_int, 1);
pub const CGPU_SINGLE_GPU_NODE_INDEX = @as(c_int, 0);
pub const CGPUSurfaceDescriptor = struct_CGPUSurfaceDescriptor;
pub const CGPUSurface = struct_CGPUSurface;
pub const CGPUDepthStateDesc = struct_CGPUDepthStateDesc;
pub const CGPUMemoryPool = struct_CGPUMemoryPool;
pub const Buffer = struct_Buffer;
pub const CGPUDStorageFile = struct_CGPUDStorageFile;
pub const HWND__ = struct_HWND__;
pub const CGPURuntimeTable = struct_CGPURuntimeTable;
pub const CGPUAdapterDescriptor = struct_CGPUAdapterDescriptor;
pub const CGPUQueueDescriptor = struct_CGPUQueueDescriptor;
pub const CGPUSemaphoreDescriptor = struct_CGPUSemaphoreDescriptor;
pub const CGPUFenceDescriptor = struct_CGPUFenceDescriptor;
pub const CGPURenderPassEncoderDescriptor = struct_CGPURenderPassEncoderDescriptor;
pub const CGPUComputePassEncoderDescriptor = struct_CGPUComputePassEncoderDescriptor;
pub const CGPUShaderReflectionDescriptor = struct_CGPUShaderReflectionDescriptor;
pub const CGPUPipelineReflectionDescriptor = struct_CGPUPipelineReflectionDescriptor;
pub const CGPUDStorageFileDescriptor = struct_CGPUDStorageFileDescriptor;