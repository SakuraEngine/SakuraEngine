#pragma once
#include "SkrAnim/resources/skin_resource.h"
#include "SkrRenderer/primitive_draw.h"
#include "SkrAnim/ozz/base/maths/simd_math.h"
#ifndef __meta__
    #include "SkrAnim/components/skin_component.generated.h"
#endif

sreflect_struct("guid" : "05B43406-4BCF-4E59-B2D8-ACED7D37E776")
sattr("component" :
{
    "custom" : "::dual::managed_component"
}) 
skr_render_skin_comp_t
{
    SKR_RESOURCE_FIELD(skr_skin_resource_t, skin_resource);
    sattr("transient": true)
    eastl::vector<uint16_t> joint_remaps;
    
    sattr("no-rtti": true, "transient": true)
    eastl::vector<ozz::math::Float4x4> skin_matrices;
};

sreflect_struct("guid" : "F9195283-41E4-4BB7-8866-5C1BDC8B51C8")
skr_skin_primitive_t 
{
    skr_vertex_buffer_entry_t position;
    skr_vertex_buffer_entry_t normal;
    skr_vertex_buffer_entry_t tangent;
    skr::span<skr_vertex_buffer_view_t> views;
};

sreflect_struct("guid" : "02753B87-0D94-4C35-B768-DE3BFE3E0DEB")
sattr("component" :
{
    "custom" : "::dual::managed_component"
}) 
skr_render_anim_comp_t
{
    ~skr_render_anim_comp_t();
    bool use_dynamic_buffer = false;
    spush_attr("no-rtti": true, "transient": true)
    eastl::vector<ozz::math::Float4x4> joint_matrices;
    eastl::vector<skr_skin_primitive_t> primitives;
    eastl::vector<skr_blob_t> buffers;
    eastl::vector<CGPUBufferId> vbs;
    eastl::vector<skr_vertex_buffer_view_t> views;
};

struct skr_render_skel_comp_t;
SKR_ANIM_API void skr_init_skin_component(skr_render_skin_comp_t* component, skr_skeleton_resource_t* skeleton);
SKR_ANIM_API void skr_init_anim_component(skr_render_anim_comp_t* component, const skr_mesh_resource_t* mesh, skr_skeleton_resource_t* skeleton);

SKR_ANIM_API void skr_cpu_skin(skr_render_skin_comp_t* skin, skr_render_anim_comp_t* anim, const skr_mesh_resource_t* mesh);