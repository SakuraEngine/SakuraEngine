#pragma once
#include "./../attributes.hxx"
#include "./../types/vec.hxx"

struct [[builtin("sampler")]] Sampler 
{

};

using SamplerState = Sampler;

template <concepts::arithmetic T, uint32 cache_flag = TextureFlags::ReadOnly>
struct [[builtin("texture1d")]] Tex1D
{
    using ElementType = T;

    [[callop("Texture1D::Size")]] uint Size();

    [[callop("Texture1D::Load")]] T Load(int location);
    [[callop("Texture1D::LoadAtMip")]] T Load(int2 location_and_mip);
    [[callop("Texture1D::LoadWithOffset")]] T Load(int2 location, int offset);
    [[callop("Texture1D::Store")]] void Store(int2 coord, T val) requires(cache_flag == TextureFlags::ReadWrite);
    [[access]] T operator[](uint location) const;
    [[access]] T& operator[](uint location);

    [[callop("Texture1D::Sample")]] T Sample(const SamplerState&, float);
    [[callop("Texture1D::SampleLevel")]] T SampleLevel(const SamplerState&, float, float);
};

template <concepts::arithmetic T, uint32 cache_flag = TextureFlags::ReadOnly>
struct [[builtin("texture2d")]] Tex2D
{
    using ElementType = T;

    [[callop("Texture2D::Size")]] uint2 Size();
    [[callop("Texture2D::Load")]] T Load(int2 location);
    [[callop("Texture2D::LoadAtMip")]] T Load(int3 location_and_mip);
    [[callop("Texture2D::LoadWithOffset")]] T Load(int3 location, int2 offset);
    [[callop("Texture2D::Store")]] void Store(int2 coord, T val) requires(cache_flag == TextureFlags::ReadWrite);
    [[access]] T operator[](int2 location) const;
    [[access]] T& operator[](int2 location);

    [[callop("Texture2D::Sample")]] T Sample(const SamplerState&, float2);
    [[callop("Texture2D::SampleLevel")]] T SampleLevel(const SamplerState&, float2, float);

    [[callop("Texture2D::Gather")]] vec<scalar_type<T>, 4> Gather(const SamplerState&, float2 location, int2 offset = int2(0));
    [[callop("Texture2D::GatherRed")]] vec<scalar_type<T>, 4> GatherRed(const SamplerState&, float2 location, int2 offset = int2(0));
    [[callop("Texture2D::GatherGreen")]] vec<scalar_type<T>, 4> GatherGreen(const SamplerState&, float2 location, int2 offset = int2(0));
    [[callop("Texture2D::GatherBlue")]] vec<scalar_type<T>, 4> GatherBlue(const SamplerState&, float2 location, int2 offset = int2(0));
    [[callop("Texture2D::GatherAlpha")]] vec<scalar_type<T>, 4> GatherAlpha(const SamplerState&, float2 location, int2 offset = int2(0));

    [[callop("Texture2D::GatherCmp")]] float4 GatherCmp(const SamplerState&, float2 location, float compare_value, int2 offset = int2(0));
    [[callop("Texture2D::GatherCmpRed")]] float4 GatherCmpRed(const SamplerState&, float2 location, float compare_value, int2 offset = int2(0));
    [[callop("Texture2D::GatherCmpGreen")]] float4 GatherCmpGreen(const SamplerState&, float2 location, float compare_value, int2 offset = int2(0));
    [[callop("Texture2D::GatherCmpBlue")]] float4 GatherCmpBlue(const SamplerState&, float2 location, float compare_value, int2 offset = int2(0));
    [[callop("Texture2D::GatherCmpAlpha")]] float4 GatherCmpAlpha(const SamplerState&, float2 location, float compare_value, int2 offset = int2(0));
};

template <concepts::arithmetic T, uint32 cache_flag = TextureFlags::ReadOnly>
struct [[builtin("texture3d")]] Tex3D
{
    using ElementType = T;
    [[callop("Texture3D::Size")]] uint3 Size();
    [[callop("Texture3D::Load")]] T Load(uint3 coord);
    [[callop("Texture3D::LoadWithOffset")]] T Load(uint3 coord, int3 offset);
    [[callop("Texture3D::Store")]] void Store(uint3 coord, T val) requires(cache_flag == TextureFlags::ReadWrite);
    [[access]] T operator[](uint3 coord) const;
    [[access]] T& operator[](uint3 coord);

    [[callop("Texture3D::Sample")]] T Sample(const SamplerState&, float3);
    [[callop("Texture3D::SampleLevel")]] T SampleLevel(const SamplerState&, float3, float);
};

template <concepts::arithmetic T, uint32 cache_flag = TextureFlags::ReadOnly>
struct [[builtin("texture1d_array")]] Tex1DArray
{
    using ElementType = T;
    [[callop("Texture1DArray::Size")]] uint2 Size();
    [[access]] T operator[](uint2 pos);

    void GetDimensions(uint& x, uint& y)
    {
        uint2 _size = Size();
        x = _size.x; y = _size.y;
    }
};

template <concepts::arithmetic T, uint32 cache_flag = TextureFlags::ReadOnly>
struct [[builtin("texture2d_array")]] Tex2DArray
{
    using ElementType = T;
    [[callop("Texture2DArray::Size")]] uint3 Size();
    [[access]] T operator[](uint3 pos);

    void GetDimensions(uint& x, uint& y, uint& z)
    {
        uint3 _size = Size();
        x = _size.x; y = _size.y; z = _size.z;
    }
};

template <concepts::arithmetic T, uint32 cache_flag = TextureFlags::ReadOnly>
struct [[builtin("texture3d_array")]] Tex3DArray
{
    static_assert(cache_flag != TextureFlags::ReadWrite);
    using ElementType = T;
    
};

template <concepts::arithmetic T, uint32 cache_flag = TextureFlags::ReadOnly>
struct [[builtin("texture_cube")]] TexCube
{
    static_assert(cache_flag != TextureFlags::ReadWrite);
    using ElementType = T;

    [[callop("TextureCube::Sample")]] T Sample(const SamplerState&, float3);
    [[callop("TextureCube::SampleLevel")]] T SampleLevel(const SamplerState&, float3, float);
};

template <concepts::arithmetic T = float4>
using Texture1D = Tex1D<T, TextureFlags::ReadOnly>;

template <concepts::arithmetic T = float4>
using RWTexture1D = Tex1D<T, TextureFlags::ReadWrite>;

template <concepts::arithmetic T = float4>
using Texture1DArray = Tex1DArray<T, TextureFlags::ReadOnly>;

template <concepts::arithmetic T = float4>
using RWTexture1DArray = Tex1DArray<T, TextureFlags::ReadWrite>;

template <concepts::arithmetic T = float4>
using Texture2D = Tex2D<T, TextureFlags::ReadOnly>;

template <concepts::arithmetic T = float4>
using RWTexture2D = Tex2D<T, TextureFlags::ReadWrite>;

template <concepts::arithmetic T = float4>
using Texture2DArray = Tex2DArray<T, TextureFlags::ReadOnly>;

template <concepts::arithmetic T = float4>
using RWTexture2DArray = Tex2DArray<T, TextureFlags::ReadWrite>;

template <concepts::arithmetic T = float4>
using Texture3D = Tex3D<T, TextureFlags::ReadOnly>;

template <concepts::arithmetic T = float4>
using RWTexture3D = Tex3D<T, TextureFlags::ReadWrite>;

template <concepts::arithmetic T = float4>
using Texture3DArray = Tex3DArray<T, TextureFlags::ReadOnly>;

template <concepts::arithmetic T = float4>
using TextureCube = TexCube<T, TextureFlags::ReadOnly>;

template <concepts::arithmetic T = float4>
using TextureCubeArray = Tex3DArray<T, TextureFlags::ReadOnly>;