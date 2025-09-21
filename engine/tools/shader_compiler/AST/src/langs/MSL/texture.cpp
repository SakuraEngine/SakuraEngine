namespace skr::CppSL::MSL
{
const wchar_t* kMSLTextureIntrinsics = LR"LR(
template <typename T, metal::access a = metal::access::sample> struct Texture1D { metal::texture1d<typename __ScalarType<T>::Type, a> cgpu_texture; };
template <typename T> using RWTexture1D = Texture1D<typename __ScalarType<T>::Type, metal::access::read_write>;

template <typename T, metal::access a = metal::access::sample> struct Texture2D { metal::texture2d<typename __ScalarType<T>::Type, a> cgpu_texture; };
template <typename T> using RWTexture2D = Texture2D<typename __ScalarType<T>::Type, metal::access::read_write>;

template <typename T, metal::access a = metal::access::sample> struct Texture2DArray { metal::texture2d_array<typename __ScalarType<T>::Type, a> cgpu_texture; };
template <typename T> using RWTexture2DArray = Texture2DArray<typename __ScalarType<T>::Type, metal::access::read_write>;

template <typename T, metal::access a = metal::access::sample> struct TextureCube { metal::texturecube<typename __ScalarType<T>::Type, a> cgpu_texture; };
template <typename T> using RWTextureCube = TextureCube<typename __ScalarType<T>::Type, metal::access::read_write>;

template <typename T, metal::access a = metal::access::sample> struct Texture3D { metal::texture3d<typename __ScalarType<T>::Type, a> cgpu_texture; };
template <typename T> using RWTexture3D = Texture3D<typename __ScalarType<T>::Type, metal::access::read_write>;

// Subscript wrapper for texture operations
template<typename TEX, typename ScalarType, typename Coord>
struct CppSLTextureSubscript {
    TEX texture;
    Coord coord;
    operator metal::vec<ScalarType, 4>() const { return texture.read(coord); }
    operator metal::vec<ScalarType, 3>() const { return texture.read(coord).xyz; }
    operator metal::vec<ScalarType, 2>() const { return texture.read(coord).xy; }
    operator ScalarType() const { return texture.read(coord).x; }
    void operator=(metal::vec<ScalarType, 4> value) { texture.write(value, coord); }
    void operator+=(metal::vec<ScalarType, 4> value) { texture.write(texture.read(coord) + value, coord); }
    void operator-=(metal::vec<ScalarType, 4> value) { texture.write(texture.read(coord) - value, coord); }
    void operator*=(metal::vec<ScalarType, 4> value) { texture.write(texture.read(coord) * value, coord); }
    void operator/=(metal::vec<ScalarType, 4> value) { texture.write(texture.read(coord) / value, coord); }
};
template<typename TEX, typename ScalarType, typename Coord>
struct CppSLTextureArraySubscript {
    TEX texture;
    Coord coord;
    uint slice;
    operator metal::vec<ScalarType, 4>() const { return texture.read(coord, slice); }
    void operator=(metal::vec<ScalarType, 4> value) { texture.write(value, coord, slice); }
    void operator+=(metal::vec<ScalarType, 4> value) { texture.write(texture.read(coord, slice) + value, coord, slice); }
    void operator-=(metal::vec<ScalarType, 4> value) { texture.write(texture.read(coord, slice) - value, coord, slice); }
    void operator*=(metal::vec<ScalarType, 4> value) { texture.write(texture.read(coord, slice) * value, coord, slice); }
    void operator/=(metal::vec<ScalarType, 4> value) { texture.write(texture.read(coord, slice) / value, coord, slice); }
};

// Helper functions to create subscript wrappers
template<typename T, metal::access a> auto subscript_wrapper(Texture1D<T, a> texture, uint coord) { return CppSLTextureSubscript<decltype(texture.cgpu_texture), typename __ScalarType<T>::Type, uint>{ texture.cgpu_texture, coord }; }
template<typename T, metal::access a> auto subscript_wrapper(Texture2D<T, a> texture, uint2 coord) { return CppSLTextureSubscript<decltype(texture.cgpu_texture), typename __ScalarType<T>::Type, uint2>{ texture.cgpu_texture, coord }; }
template<typename T, metal::access a> auto subscript_wrapper(Texture3D<T, a> texture, uint3 coord) { return CppSLTextureSubscript<decltype(texture.cgpu_texture), typename __ScalarType<T>::Type, uint3>{ texture.cgpu_texture, coord }; }

template<typename T, metal::access a> auto subscript_wrapper(Texture2DArray<T, a> texture, uint3 coord) { return CppSLTextureArraySubscript<decltype(texture.cgpu_texture), typename __ScalarType<T>::Type, uint2>{ texture.cgpu_texture, coord.xy, coord.z }; }
template<typename T, metal::access a> auto subscript_wrapper(TextureCube<T, a> texture, uint3 coord) { return CppSLTextureSubscript<decltype(texture.cgpu_texture), typename __ScalarType<T>::Type, uint3>{ texture.cgpu_texture, coord }; }

struct SamplerState { metal::sampler cgpu_sampler; };
)LR"
LR"LR(
// Texture1D functions
template<typename T, metal::access a>
metal::uint CppSLTextureSize(Texture1D<T, a> texture) {
    return texture.cgpu_texture.get_width();
}
template<typename T>
auto CppSLTextureSample(Texture1D<T, metal::access::sample> texture, SamplerState sampler, float uv) { 
    return texture.cgpu_texture.sample(sampler.cgpu_sampler, uv); 
}
template<typename T>
auto CppSLTextureSampleLevel(Texture1D<T, metal::access::sample> texture, SamplerState sampler, float uv, float lv) { 
    return texture.cgpu_texture.sample(sampler.cgpu_sampler, uv); 
}
template<typename T, metal::access a>
auto CppSLTextureLoad(Texture1D<T, a> texture, uint coord) {
    return texture.cgpu_texture.read(coord);
}
template<typename T, metal::access a>
auto CppSLTextureLoad(Texture1D<T, a> texture, uint2 coord_lod) {
    return texture.cgpu_texture.read(coord_lod.x, coord_lod.y);
}
template<typename T, typename V>
void CppSLTextureStore(Texture1D<T, metal::access::read_write> texture, uint coord, V value) {
    texture.cgpu_texture.write(value, coord);
}

// Texture2D functions
template<typename T, metal::access a>
metal::uint2 CppSLTextureSize(Texture2D<T, a> texture) {
    return metal::uint2(texture.cgpu_texture.get_width(), texture.cgpu_texture.get_height()); 
}
template<typename T>
auto CppSLTextureSample(Texture2D<T, metal::access::sample> texture, SamplerState sampler, float2 uv) { 
    return texture.cgpu_texture.sample(sampler.cgpu_sampler, uv); 
}
template<typename T>
auto CppSLTextureSampleLevel(Texture2D<T, metal::access::sample> texture, SamplerState sampler, float2 uv, float lv) { 
    return texture.cgpu_texture.sample(sampler.cgpu_sampler, uv, metal::level(lv)); 
}
template<typename T, metal::access a>
auto CppSLTextureLoad(Texture2D<T, a> texture, uint2 coord) {
    return texture.cgpu_texture.read(coord);
}
template<typename T, metal::access a>
auto CppSLTextureLoad(Texture2D<T, a> texture, uint3 coord_lod) {
    return texture.cgpu_texture.read(coord_lod.xy, coord_lod.z);
}
template<typename T, typename V>
void CppSLTextureStore(Texture2D<T, metal::access::read_write> texture, uint2 coord, V value) {
    texture.cgpu_texture.write(value, coord);
}

// Texture2DArray functions
template<typename T, metal::access a>
metal::uint3 CppSLTextureSize(Texture2DArray<T, a> texture) {
    return metal::uint3(texture.cgpu_texture.get_width(), texture.cgpu_texture.get_height(), texture.cgpu_texture.get_array_size()); 
}
template<typename T>
auto CppSLTextureSample(Texture2DArray<T, metal::access::sample> texture, SamplerState sampler, float3 uv) { 
    return texture.cgpu_texture.sample(sampler.cgpu_sampler, uv.xy, metal::level(uv.z)); 
}
template<typename T>
auto CppSLTextureSampleLevel(Texture2DArray<T, metal::access::sample> texture, SamplerState sampler, float3 uv, float lv) { 
    return texture.cgpu_texture.sample(sampler.cgpu_sampler, uv.xy, metal::level(lv), uv.z); 
}
template<typename T, metal::access a>
auto CppSLTextureLoad(Texture2DArray<T, a> texture, uint3 coord) {
    return texture.cgpu_texture.read(coord.xy, coord.z);
}
template<typename T, metal::access a>
auto CppSLTextureLoad(Texture2DArray<T, a> texture, uint4 coord_lod) {
    return texture.cgpu_texture.read(coord_lod.xy, coord_lod.z, coord_lod.w);
}
template<typename T, typename V>
void CppSLTextureStore(Texture2DArray<T, metal::access::read_write> texture, uint3 coord, V value) {
    texture.cgpu_texture.write(value, coord.xy, coord.z);
}

// TextureCube functions
template<typename T, metal::access a>
metal::uint2 CppSLTextureSize(TextureCube<T, a> texture) {
    return metal::uint2(texture.cgpu_texture.get_width(), texture.cgpu_texture.get_height()); 
}
template<typename T>
auto CppSLTextureSample(TextureCube<T, metal::access::sample> texture, SamplerState sampler, float3 direction) { 
    return texture.cgpu_texture.sample(sampler.cgpu_sampler, direction); 
}
template<typename T>
auto CppSLTextureSampleLevel(TextureCube<T, metal::access::sample> texture, SamplerState sampler, float3 direction, float lv) { 
    return texture.cgpu_texture.sample(sampler.cgpu_sampler, direction, metal::level(lv)); 
}
template<typename T, metal::access a>
auto CppSLTextureLoad(TextureCube<T, a> texture, uint3 coord) {
    return texture.cgpu_texture.read(coord.xy, coord.z);
}
template<typename T, metal::access a>
auto CppSLTextureLoad(TextureCube<T, a> texture, uint4 coord_lod) {
    return texture.cgpu_texture.read(coord_lod.xy, coord_lod.z, coord_lod.w);
}
template<typename T, typename V>
void CppSLTextureStore(TextureCube<T, metal::access::read_write> texture, uint3 coord, V value) {
    texture.cgpu_texture.write(value, coord.xy, coord.z);
}

// Texture3D functions
template<typename T, metal::access a>
metal::uint3 CppSLTextureSize(Texture3D<T, a> texture) {
    return metal::uint3(texture.cgpu_texture.get_width(), texture.cgpu_texture.get_height(), texture.cgpu_texture.get_depth()); 
}
template<typename T>
auto CppSLTextureSample(Texture3D<T, metal::access::sample> texture, SamplerState sampler, float3 uv) { 
    return texture.cgpu_texture.sample(sampler.cgpu_sampler, uv); 
}
template<typename T>
auto CppSLTextureSampleLevel(Texture3D<T, metal::access::sample> texture, SamplerState sampler, float3 uv, float lv) { 
    return texture.cgpu_texture.sample(sampler.cgpu_sampler, uv, metal::level(lv)); 
}
template<typename T, metal::access a>
auto CppSLTextureLoad(Texture3D<T, a> texture, uint3 coord) {
    return texture.cgpu_texture.read(coord);
}
template<typename T, metal::access a>
auto CppSLTextureLoad(Texture3D<T, a> texture, uint4 coord_lod) {
    return texture.cgpu_texture.read(coord_lod.xyz, coord_lod.w);
}
template<typename T, typename V>
void CppSLTextureStore(Texture3D<T, metal::access::read_write> texture, uint3 coord, V value) {
    texture.cgpu_texture.write(value, coord);
}
)LR";

} // namespace CppSL::MSL