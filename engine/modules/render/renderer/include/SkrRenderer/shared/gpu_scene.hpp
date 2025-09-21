#pragma once
#include "SkrRenderer/shared/gpu_table.hpp"
#if !defined(__CPPSL__)
    #include "SkrRenderer/shared/gpu_scene.generated.h" // IWYU pragma: export
#endif

namespace skr::gpu
{

struct [[secs_managed_component, sattr(guid = "74e60480-7324-48d7-a174-bc5e404fe0bc" gpu.soa=@enable)]]
Material
{
    uint32_t basecolor_tex;
    uint32_t normal_tex;
    uint32_t metallic_roughness_tex;
    uint32_t emission_tex;
    uint32_t global_index;
};

struct [[secs_managed_component, sattr(guid = "62af1ab3-7762-4413-97f9-fab30d9232c0" gpu.soa=@enable)]]
Primitive
{
    Row<Material> material;
    Range<uint> indices;
    Range<float3> positions;
    Range<float3> normals;
    Range<float3> tangents;
    Range<float2> uvs;
    uint32_t global_index;
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
    Range<Primitive> primitives;
    uint32_t global_index;
};

} // namespace skr::gpu

#include "SkrRenderer/shared/gpu_scene.datablocks.generated.h" // IWYU pragma: export