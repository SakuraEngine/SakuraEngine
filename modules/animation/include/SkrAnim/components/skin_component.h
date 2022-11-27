#pragma once
#include "SkrAnim/resources/skin_resource.h"
#ifndef __meta__
    #include "SkrAnim/components/skin_component.generated.h"
#endif

sreflect_struct("guid" : "05B43406-4BCF-4E59-B2D8-ACED7D37E776")
sattr("component" :
{
    "custom" : "::dual::managed_component"
}) 
skr_skin_component_t
{
    SKR_RESOURCE_FIELD(skr_skin_resource_t, skin);
    eastl::vector<uint16_t> joint_remaps;
};

sreflect_struct("guid" : "02753B87-0D94-4C35-B768-DE3BFE3E0DEB")
sattr("component" :
{
    "custom" : "::dual::managed_component"
}) 
skr_anim_component_t
{
    eastl::vector<ozz::math::Float4x4> skin_matrices;
};

struct skr_skeleton_component_t;
SKR_ANIM_API void skr_initialize_skin_component(skr_skin_component_t* component, skr_skeleton_component_t* skeleton);