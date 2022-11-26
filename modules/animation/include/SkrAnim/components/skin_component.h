#pragma once
#include "SkrAnim/resources/skin_resource.h"
#ifndef __meta__
    #include "SkrAnim/components/skin_component.generated.h"
#endif

sreflect_struct("guid"
                : "05B43406-4BCF-4E59-B2D8-ACED7D37E776")
sattr("component" :
{
    "custom" : "::dual::managed_component"
}) skr_skin_component_t
{
    SKR_RESOURCE_FIELD(skr_skin_resource_t, skin);
    eastl::vector<uint16_t> joint_remaps;
};

struct skr_skeleton_component_t;
SKR_ANIM_API void skr_initialize_skin_component(skr_skin_component_t* component, skr_skeleton_component_t* skeleton);