#pragma once
#include "SkrAnim/resources/skeleton_resource.h"
#ifndef __meta__
    #include "SkrAnim/components/skeleton_component.generated.h" // IWYU pragma: export
#endif

sreflect_struct("guid": "05622CB2-9D73-402B-B6C5-8075E13D5063", "component" : true)
skr_render_skel_comp_t
{
    SKR_RESOURCE_FIELD(skr::anim::SkeletonResource, skeleton);
};