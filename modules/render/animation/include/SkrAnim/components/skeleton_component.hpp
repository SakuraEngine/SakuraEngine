#pragma once
#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrRT/ecs/sugoi_meta.hpp"
#ifndef __meta__
    #include "SkrAnim/components/skeleton_component.generated.h" // IWYU pragma: export
#endif

namespace skr::anim
{

sreflect_managed_component("guid" : "05622CB2-9D73-402B-B6C5-8075E13D5063")
SkeletonComponent {
    SKR_RESOURCE_FIELD(skr::anim::SkeletonResource, skeleton);
};

} // namespace skr::anim