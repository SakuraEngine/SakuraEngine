namespace skr::CppSL::HLSL
{
const wchar_t* kHLSLTextureIntrinsics = LR"__de___l___im__(
template <typename T> T CppSLTextureLoad(Texture2D<T> tex, uint2 loc) { return tex.Load(uint3(loc, 0)); }
template <typename T> T CppSLTextureLoad(RWTexture2D<T> tex, uint2 loc) { return tex.Load(uint3(loc, 0)); }
template <typename T> T CppSLTextureLoad(Texture2D<T> tex, uint3 loc_and_mip) { return tex.Load(loc_and_mip); }
template <typename T> T CppSLTextureLoad(RWTexture2D<T> tex, uint3 loc_and_mip) { return tex.Load(loc_and_mip); }
template <typename T, typename U> void CppSLTextureStore(RWTexture2D<T> tex, uint2 uv, U v) { tex[uv] = v; }

template <typename T> uint CppSLTextureSize(Texture1D<T> tex) { uint Width, Mips; tex.GetDimensions(0, Width, Mips); return Width; }
template <typename T> uint CppSLTextureSize(RWTexture1D<T> tex) { uint Width; tex.GetDimensions(Width); return Width; }
template <typename T> uint2 CppSLTextureSize(Texture1DArray<T> tex) { uint Width, Length; tex.GetDimensions(Width, Length); return uint2(Width, Length); }
template <typename T> uint2 CppSLTextureSize(RWTexture1DArray<T> tex) { uint Width, Length; tex.GetDimensions(Width, Length); return uint2(Width, Length); }

template <typename T> uint2 CppSLTextureSize(Texture2D<T> tex) { uint Width, Height, Mips; tex.GetDimensions(0, Width, Height, Mips); return uint2(Width, Height); }
template <typename T> uint2 CppSLTextureSize(RWTexture2D<T> tex) { uint Width, Height; tex.GetDimensions(Width, Height); return uint2(Width, Height); }
template <typename T> uint3 CppSLTextureSize(Texture2DArray<T> tex) { uint Width, Height, Length; tex.GetDimensions(Width, Height, Length); return uint3(Width, Height, Length); }
template <typename T> uint3 CppSLTextureSize(RWTexture2DArray<T> tex) { uint Width, Height, Length; tex.GetDimensions(Width, Height, Length); return uint3(Width, Height, Length); }

template <typename T> uint3 CppSLTextureSize(Texture3D<T> tex) { uint Width, Height, Depth, Mips; tex.GetDimensions(0, Width, Height, Depth, Mips); return uint3(Width, Height, Depth); }
template <typename T> uint3 CppSLTextureSize(RWTexture3D<T> tex) { uint Width, Height, Depth; tex.GetDimensions(Width, Height, Depth); return uint3(Width, Height, Depth); }

#if defined(CPPSL_COMPUTE) || defined(CPPSL_FRAGMENT)
template <typename T> T CppSLTextureSample(Texture1D<T> t, SamplerState s, float uv) { return t.Sample(s, uv); }
template <typename T> T CppSLTextureSample(Texture2D<T> t, SamplerState s, float2 uv) { return t.Sample(s, uv); }
template <typename T> T CppSLTextureSample(Texture3D<T> t, SamplerState s, float3 uv) { return t.Sample(s, uv); }
template <typename T> T CppSLTextureSample(TextureCube<T> t, SamplerState s, float3 uv) { return t.Sample(s, uv); }
#else
template <typename T> T CppSLTextureSample(Texture1D<T> t, SamplerState s, float uv) { return t.SampleLevel(s, uv, 0); }
template <typename T> T CppSLTextureSample(Texture2D<T> t, SamplerState s, float2 uv) { return t.SampleLevel(s, uv, 0); }
template <typename T> T CppSLTextureSample(Texture3D<T> t, SamplerState s, float3 uv) { return t.SampleLevel(s, uv, 0); }
template <typename T> T CppSLTextureSample(TextureCube<T> t, SamplerState s, float3 uv) { return t.SampleLevel(s, uv, 0); }
#endif

template <typename T> T CppSLTextureSampleLevel(Texture1D<T> t, SamplerState s, float uv, float level) { return t.SampleLevel(s, uv, level); }
template <typename T> T CppSLTextureSampleLevel(Texture2D<T> t, SamplerState s, float2 uv, float level) { return t.SampleLevel(s, uv, level); }
template <typename T> T CppSLTextureSampleLevel(Texture3D<T> t, SamplerState s, float3 uv, float level) { return t.SampleLevel(s, uv, level); }
template <typename T> T CppSLTextureSampleLevel(TextureCube<T> t, SamplerState s, float3 uv, float level) { return t.SampleLevel(s, uv, level); }
)__de___l___im__";

} // namespace CppSL::HLSL