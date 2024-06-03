#pragma once
#include "SkrBase/config.h"
#include "SkrRT/config.h"
#ifndef __meta__
    #include "SkrAnim/resources/skeleton_resource.generated.h" // IWYU pragma: export
#endif

#ifdef __cplusplus
    #include "SkrAnim/ozz/skeleton.h"
    #include "SkrBase/types.h"
    #include "SkrBase/types.h"
    #include "SkrRT/resource/resource_factory.h"

namespace skr sreflect
{
namespace anim sreflect
{

sreflect_struct("guid": "1876BF35-E4DC-450B-B9D4-09259397F4BA")
SkeletonResource {
    ozz::animation::Skeleton skeleton;
};
} // namespace anim sreflect

namespace binary
{
template <>
struct SKR_ANIM_API ReadTrait<skr::anim::SkeletonResource> {
    static bool Read(SBinaryReader* reader, skr::anim::SkeletonResource& value);
};
template <>
struct SKR_ANIM_API WriteTrait<skr::anim::SkeletonResource> {
    static bool Write(SBinaryWriter* writer, const skr::anim::SkeletonResource& value);
};
} // namespace binary

namespace resource
{
struct SKR_ANIM_API SSkelFactory : public SResourceFactory {
public:
    virtual ~SSkelFactory() noexcept = default;
    skr_guid_t GetResourceType() override;
    bool       AsyncIO() override { return true; }
};
} // namespace resource
} // namespace skr sreflect
#endif