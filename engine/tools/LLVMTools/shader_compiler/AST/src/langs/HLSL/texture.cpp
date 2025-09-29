namespace skr::CppSL::HLSL
{
const wchar_t* kHLSLTextureIntrinsics = LR"__de___l___im__(
#define CppSLTexture2DLoad(tex, loc) (tex).Load(int3((loc), 0))
#define CppSLTexture2DLoadAtMip(tex, loc_and_mip) (tex).Load((loc_and_mip))
#define CppSLTexture2DLoadWithOffset(tex, loc_and_mip, offset) (tex).Load((loc_and_mip), (offset))

#define CppSLTexture1DStore(tex, uv, v) (tex)[(uv)] = (v)
#define CppSLTexture2DStore(tex, uv, v) (tex)[(uv)] = (v)
#define CppSLTexture3DStore(tex, uv, v) (tex)[(uv)] = (v)

template <typename T1D>  uint  CppSLTexture1DSize(T1D tex) { uint Width, Mips; tex.GetDimensions(0, Width, Mips); return Width; }
template <typename T1DA> uint2 CppSLTexture1DArraySize(T1DA tex) { uint Width, Length; tex.GetDimensions(Width, Length); return uint2(Width, Length); }
template <typename T2D>  uint2 CppSLTexture2DSize(T2D tex) { uint Width, Height; tex.GetDimensions(Width, Height); return uint2(Width, Height); }
template <typename T2DA> uint3 CppSLTexture2DArraySize(T2DA tex) { uint Width, Height, Length; tex.GetDimensions(Width, Height, Length); return uint3(Width, Height, Length); }
template <typename T3D>  uint3 CppSLTexture3DSize(T3D tex) { uint Width, Height, Depth, Mips; tex.GetDimensions(0, Width, Height, Depth, Mips); return uint3(Width, Height, Depth); }

#if defined(CPPSL_COMPUTE) || defined(CPPSL_FRAGMENT)
#define CppSLTexture1DSample(t, s, uv) t.Sample((s), (uv))
#define CppSLTexture2DSample(t, s, uv) t.Sample((s), (uv))
#define CppSLTexture3DSample(t, s, uv) t.Sample((s), (uv))
#define CppSLTextureCubeSample(t, s, uv) t.Sample((s), (uv))
#else
#define CppSLTexture1DSample(t, s, uv) t.SampleLevel((s), (uv), 0)
#define CppSLTexture2DSample(t, s, uv) t.SampleLevel((s), (uv), 0)
#define CppSLTexture3DSample(t, s, uv) t.SampleLevel((s), (uv), 0)
#define CppSLTextureCubeSample(t, s, uv) t.SampleLevel((s), (uv), 0)
#endif

#define CppSLTexture1DSampleLevel(t, s, uv, level) (t).SampleLevel((s), (uv), (level))
#define CppSLTexture2DSampleLevel(t, s, uv, level) (t).SampleLevel((s), (uv), (level))
#define CppSLTexture3DSampleLevel(t, s, uv, level) (t).SampleLevel((s), (uv), (level))
#define CppSLTextureCubeSampleLevel(t, s, uv, level) (t).SampleLevel((s), (uv), (level))

#define CppSLTexture2DGather(t, s, uv, offset) (t).Gather((s), (uv), (offset))
#define CppSLTexture2DGatherRed(t, s, uv, offset) (t).GatherRed((s), (uv), (offset))
#define CppSLTexture2DGatherGreen(t, s, uv, offset) (t).GatherGreen((s), (uv), (offset))
#define CppSLTexture2DGatherBlue(t, s, uv, offset) (t).GatherBlue((s), (uv), (offset))
#define CppSLTexture2DGatherAlpha(t, s, uv, offset) (t).GatherAlpha((s), (uv), (offset))

#define CppSLTexture2DGatherCmp(t, s, uv, cmp, offset) (t).GatherCmp((s), (uv), (cmp), (offset))
#define CppSLTexture2DGatherCmpRed(t, s, uv, cmp, offset) (t).GatherCmpRed((s), (uv), (cmp), (offset))
#define CppSLTexture2DGatherCmpGreen(t, s, uv, cmp, offset) (t).GatherCmpGreen((s), (uv), (cmp), (offset))
#define CppSLTexture2DGatherCmpBlue(t, s, uv, cmp, offset) (t).GatherCmpBlue((s), (uv), (cmp), (offset))
#define CppSLTexture2DGatherCmpAlpha(t, s, uv, cmp, offset) (t).GatherCmpAlpha((s), (uv), (cmp), (offset))

)__de___l___im__";

} // namespace CppSL::HLSL