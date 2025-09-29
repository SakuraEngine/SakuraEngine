#pragma once
#include "SkrAnim/ozz/skeleton.h"
#include "SkrBase/types.h"
#include "SkrBase/types.h"
#include "SkrRuntime/resource/resource_factory.hpp"
#include "SkrAnim/resources/skeleton_resource.generated.h" // IWYU pragma: export

namespace skr
{
struct [[sattr(guid = "1876BF35-E4DC-450B-B9D4-09259397F4BA")]]
SkeletonResource
{
    ozz::animation::Skeleton skeleton;
};

template <>
struct SKR_ANIM_API Serialize<skr::SkeletonResource>
{
    static void read(skr::ArchiveRead& r, skr::SkeletonResource& v);
    static void write(skr::ArchiveWrite& w, const skr::SkeletonResource& v);
};

struct SKR_ANIM_API SkelFactory : public ResourceFactory
{
public:
    virtual ~SkelFactory() noexcept = default;
    GUID GetResourceType() override;
    bool AsyncIO() override { return true; }
};
} // namespace skr