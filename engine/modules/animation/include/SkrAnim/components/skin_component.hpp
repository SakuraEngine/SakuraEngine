#pragma once
#include "SkrRuntime/sugoi/sugoi_meta.hpp"
#include "SkrRenderer/primitive_draw.h"
#include "SkrAnim/resources/skin_resource.hpp"
#include "SkrAnim/ozz/base/maths/simd_math.h"
#include "SkrAnim/components/skin_component.generated.h" // IWYU pragma: export

namespace skr
{

struct [[secs_component, sattr(
    guid = "05B43406-4BCF-4E59-B2D8-ACED7D37E776"; 
    serde = @enable
)]] SKR_ANIM_API SkinComponent
{
    skr::AsyncResource<skr::SkinResource> skin_resource;

    [[sattr(serde = @disable)]]
    skr::Vector<uint16_t> joint_remaps;

    [[sattr(serde = @disable)]]
    skr::Vector<ozz::math::Float4x4> skin_matrices;
};

struct [[sattr(
    guid = "F9195283-41E4-4BB7-8866-5C1BDC8B51C8"; 
    serde = @enable
)]] SKR_ANIM_API SkinPrimitive
{
    VertexBufferEntry position;
    VertexBufferEntry normal;
    VertexBufferEntry tangent;
    [[sattr(serde = @disable)]]
    skr::Span<skr_vertex_buffer_view_t> views;
};

struct [[secs_component, sattr(
    guid = "02753B87-0D94-4C35-B768-DE3BFE3E0DEB"; 
    serde = @enable
)]] SKR_ANIM_API AnimComponent
{
    ~AnimComponent();
    bool use_dynamic_buffer = false;

    [[sattr(serde = @disable)]]
    skr::Vector<ozz::math::Float4x4> joint_matrices;
    [[sattr(serde = @disable)]]
    skr::Vector<skr::SkinPrimitive> primitives;
    [[sattr(serde = @disable)]]
    skr::Vector<RC<skr::IBlob>> buffers;
    [[sattr(serde = @disable)]]
    skr::Vector<CGPUBufferId> vbs;
    [[sattr(serde = @disable)]]
    skr::Vector<skr_vertex_buffer_view_t> views;
};

} // namespace skr

SKR_ANIM_API void skr_init_skin_component(skr::SkinComponent* component, const skr::SkeletonResource* skeleton);
SKR_ANIM_API void skr_init_anim_component(skr::AnimComponent* component, const skr::MeshResource* mesh, skr::SkeletonResource* skeleton);
SKR_ANIM_API void skr_init_anim_buffers(CGPUDeviceId device, skr::AnimComponent* anim, const skr::MeshResource* mesh);

SKR_ANIM_API void skr_cpu_skin(skr::SkinComponent* skin, const skr::AnimComponent* anim, const skr::MeshResource* mesh);