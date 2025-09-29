#pragma once
#include "SkrRenderer/shared/gpu_table.hpp"
#if !defined(__CPPSL__)
    #include "SkrRenderer/shared/gpu_scene.generated.h" // IWYU pragma: export
#endif

namespace skr::gpu
{

struct [[sattr(guid = "74e60480-7324-48d7-a174-bc5e404fe0bc" gpu.soa=@enable)]]
PBRMaterial
{
    float3 basecolor;
    float roughness;
    float metallic;
    uint32_t basecolor_tex;
    uint32_t normal_tex;
    uint32_t metallic_roughness_tex;
    uint32_t emission_tex;
    uint32_t global_index;
};

struct [[sattr(guid = "62af1ab3-7762-4413-97f9-fab30d9232c0" gpu.soa=@enable)]]
Primitive
{
    uint32_t material_index;
    Range<uint> indices;
    Range<float3> positions;
    Range<float3> normals;
    Range<float3> tangents;
    Range<float2> uvs;
    uint32_t global_index;
};

struct [[sattr(guid = "b757d92b-5b05-416d-a347-30beb57a0ad9" gpu.aos=@enable)]]
MaterialID
{
    uint32_t type;
    uint32_t index;
};

struct [[sattr(guid = "1825c546-7bb8-4b52-8a20-76b3b4bbe542" gpu.soa=@enable), sattr(ecs.comp.array = 1)]]
Instance
{
    float4x4 transform = float4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
    Range<MaterialID> materials;
    Range<Primitive> primitives;
    uint32_t global_index;
};

} // namespace skr::gpu

#include "SkrRenderer/shared/gpu_scene.datablocks.generated.h" // IWYU pragma: export