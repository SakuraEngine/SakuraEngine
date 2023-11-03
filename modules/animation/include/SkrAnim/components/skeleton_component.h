#pragma once
#include "SkrAnim/resources/skeleton_resource.h"
#ifndef __meta__
    #include "SkrAnim/components/skeleton_component.generated.h" // IWYU pragma: export
#endif

namespace skr sreflect {
namespace anim sreflect {

sreflect_struct("guid": "05622CB2-9D73-402B-B6C5-8075E13D5063", "component" : true)
SkeletonComponent
{
    SKR_RESOURCE_FIELD(skr::anim::SkeletonResource, skeleton);
};

} // namespace anim
} // namespace skr