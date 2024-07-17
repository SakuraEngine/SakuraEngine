#pragma once
#include "SkrRT/ecs/sugoi_meta.hpp"
#include "SkrRenderer/primitive_draw.h"
#include "SkrAnim/resources/skin_resource.hpp"
#include "SkrAnim/ozz/base/maths/simd_math.h"
#ifndef __meta__
    #include "SkrAnim/components/skin_component.generated.h" // IWYU pragma: export
#endif

namespace skr::anim
{

sreflect_managed_component("guid" : "05B43406-4BCF-4E59-B2D8-ACED7D37E776")
SkinComponent {
    SKR_RESOURCE_FIELD(SkinResource, skin_resource);
    sattr("serde": "disable")
    skr::Vector<uint16_t>            joint_remaps;

    sattr("serde": "disable")
    skr::Vector<ozz::math::Float4x4> skin_matrices;
};

sreflect_struct("guid" : "F9195283-41E4-4BB7-8866-5C1BDC8B51C8")
SkinPrimitive {
    skr_vertex_buffer_entry_t           position;
    skr_vertex_buffer_entry_t           normal;
    skr_vertex_buffer_entry_t           tangent;
    skr::span<skr_vertex_buffer_view_t> views;
};

sreflect_managed_component("guid" : "02753B87-0D94-4C35-B768-DE3BFE3E0DEB")
AnimComponent {
    ~AnimComponent();
    bool                                  use_dynamic_buffer = false;

    sattr("serde": "disable")
    skr::Vector<ozz::math::Float4x4>      joint_matrices;
    sattr("serde": "disable")
    skr::Vector<skr::anim::SkinPrimitive> primitives;
    sattr("serde": "disable")
    skr::Vector<skr::IBlob*>              buffers;
    sattr("serde": "disable")
    skr::Vector<CGPUBufferId>             vbs;
    sattr("serde": "disable")
    skr::Vector<skr_vertex_buffer_view_t> views;
};

} // namespace skr::anim

SKR_ANIM_API void skr_init_skin_component(skr::anim::SkinComponent* component, const skr::anim::SkeletonResource* skeleton);
SKR_ANIM_API void skr_init_anim_component(skr::anim::AnimComponent* component, const skr_mesh_resource_t* mesh, skr::anim::SkeletonResource* skeleton);
SKR_ANIM_API void skr_init_anim_buffers(CGPUDeviceId device, skr::anim::AnimComponent* anim, const skr_mesh_resource_t* mesh);

SKR_ANIM_API void skr_cpu_skin(skr::anim::SkinComponent* skin, const skr::anim::AnimComponent* anim, const skr_mesh_resource_t* mesh);