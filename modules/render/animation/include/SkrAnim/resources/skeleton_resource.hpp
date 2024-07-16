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

namespace skr
{
namespace anim
{

sreflect_struct("guid": "1876BF35-E4DC-450B-B9D4-09259397F4BA")
SkeletonResource {
    ozz::animation::Skeleton skeleton;
};
} // namespace anim

template <>
struct SKR_ANIM_API BinSerde<skr::anim::SkeletonResource> {
    static bool read(SBinaryReader* r, skr::anim::SkeletonResource& v);
    static bool write(SBinaryWriter* w, const skr::anim::SkeletonResource& v);
};

namespace resource
{
struct SKR_ANIM_API SSkelFactory : public SResourceFactory {
public:
    virtual ~SSkelFactory() noexcept = default;
    skr_guid_t GetResourceType() override;
    bool       AsyncIO() override { return true; }
};
} // namespace resource
} // namespace skr
#endif