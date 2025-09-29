#pragma once
#include "SkrRuntime/resource/resource_factory.hpp"
#include "SkrBase/types.h"
#include "SkrAnim/ozz/animation.h"
#include "SkrAnim/resources/animation_resource.generated.h" // IWYU pragma: export

namespace skr
{
struct [[sattr(guid = "5D6DC46B-8696-4DD8-ADE4-C27D07CEDCCD")]]
AnimResource
{
    ozz::animation::Animation animation;
};

template <>
struct SKR_ANIM_API Serialize<AnimResource>
{
    static void read(skr::ArchiveRead& r, AnimResource& v);
    static void write(skr::ArchiveWrite& w, const AnimResource& v);
};

class SKR_ANIM_API AnimFactory : public ResourceFactory
{
public:
    virtual ~AnimFactory() noexcept = default;
    GUID GetResourceType() override;
    bool AsyncIO() override { return true; }
};
} // namespace skr