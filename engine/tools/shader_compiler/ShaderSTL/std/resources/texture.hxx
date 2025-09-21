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

    [[callop("Texture::Size")]] uint Size();

    [[callop("Texture::Load")]] T Load(uint2 coord);
    [[callop("Texture::Load")]] T Load(uint3 coord);
    [[callop("Texture::Store")]] void Store(uint2 coord, T val) requires(cache_flag == TextureFlags::ReadWrite);
    [[access]] T operator[](uint coord) const;
    [[access]] T& operator[](uint coord);

    [[callop("Texture::Sample")]] T Sample(const SamplerState&, float);
    [[callop("Texture::SampleLevel")]] T SampleLevel(const SamplerState&, float, float);
};

template <concepts::arithmetic T, uint32 cache_flag = TextureFlags::ReadOnly>
struct [[builtin("texture2d")]] Tex2D
{
    using ElementType = T;

    [[callop("Texture::Size")]] uint2 Size();
    [[callop("Texture::Load")]] T Load(uint2 coord);
    [[callop("Texture::Load")]] T Load(uint3 coord);
    [[callop("Texture::Store")]] void Store(uint2 coord, T val) requires(cache_flag == TextureFlags::ReadWrite);
    [[access]] T operator[](uint2 coord) const;
    [[access]] T& operator[](uint2 coord);

    [[callop("Texture::Sample")]] T Sample(const SamplerState&, float2);
    [[callop("Texture::SampleLevel")]] T SampleLevel(const SamplerState&, float2, float);
};

template <concepts::arithmetic T, uint32 cache_flag = TextureFlags::ReadOnly>
struct [[builtin("texture3d")]] Tex3D
{
    using ElementType = T;
    [[callop("Texture::Size")]] uint3 Size();
    [[callop("Texture::Load")]] T Load(uint3 coord);
    [[callop("Texture::Store")]] void Store(uint3 coord, T val) requires(cache_flag == TextureFlags::ReadWrite);
    [[access]] T operator[](uint3 coord) const;
    [[access]] T& operator[](uint3 coord);

    [[callop("Texture::Sample")]] T Sample(const SamplerState&, float3);
    [[callop("Texture::SampleLevel")]] T SampleLevel(const SamplerState&, float3, float);
};

template <concepts::arithmetic T, uint32 cache_flag = TextureFlags::ReadOnly>
struct [[builtin("texture1d_array")]] Tex1DArray
{
    using ElementType = T;
    [[callop("Texture::Size")]] uint2 Size();
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
    [[callop("Texture::Size")]] uint3 Size();
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

    [[callop("Texture::Sample")]] T Sample(const SamplerState&, float3);
    [[callop("Texture::SampleLevel")]] T SampleLevel(const SamplerState&, float3, float);
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