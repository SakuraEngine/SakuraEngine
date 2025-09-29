#pragma once
#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrRuntime/sugoi/sugoi_meta.hpp"
#include "SkrAnim/components/skeleton_component.generated.h" // IWYU pragma: export

namespace skr
{

struct [[secs_component, sattr(
    guid = "05622CB2-9D73-402B-B6C5-8075E13D5063";
    serde = @enable
)]] SKR_ANIM_API SkeletonComponent
{
    skr::AsyncResource<skr::SkeletonResource> skeleton_resource;
};

} // namespace skr