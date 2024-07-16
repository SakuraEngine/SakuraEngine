#pragma once
#include "SkrBase/config.h"
#include "SkrRT/config.h"
#include "SkrRT/resource/resource_factory.h"
#include "SkrBase/types.h"
#include "SkrAnim/ozz/animation.h"
#ifndef __meta__
    #include "SkrAnim/resources/animation_resource.generated.h" // IWYU pragma: export
#endif

namespace skr
{
namespace anim
{

sreflect_struct("guid": "5D6DC46B-8696-4DD8-ADE4-C27D07CEDCCD")
AnimResource {
    ozz::animation::Animation animation;
};
} // namespace anim

template <>
struct SKR_ANIM_API BinSerde<anim::AnimResource> {
    static bool read(SBinaryReader* r, anim::AnimResource& v);
    static bool write(SBinaryWriter* w, const anim::AnimResource& v);
};

namespace resource
{
class SKR_ANIM_API SAnimFactory : public SResourceFactory
{
public:
    virtual ~SAnimFactory() noexcept = default;
    skr_guid_t GetResourceType() override;
    bool       AsyncIO() override { return true; }
};
} // namespace resource
} // namespace skr