#pragma once
#include "SkrBase/config.h"
#include "SkrRT/config.h"
#include "SkrRT/resource/resource_factory.h"
#include "SkrBase/types.h"
#include "SkrAnim/ozz/animation.h"
#ifndef __meta__
    #include "SkrAnim/resources/animation_resource.generated.h" // IWYU pragma: export
#endif

namespace skr sreflect
{
namespace anim sreflect
{

sreflect_struct("guid": "5D6DC46B-8696-4DD8-ADE4-C27D07CEDCCD")
AnimResource {
    ozz::animation::Animation animation;
};
} // namespace anim sreflect

namespace binary
{
template <>
struct SKR_ANIM_API ReadTrait<anim::AnimResource> {
    static int Read(skr_binary_reader_t* reader, anim::AnimResource& value);
};
template <>
struct SKR_ANIM_API WriteTrait<anim::AnimResource> {
    static int Write(skr_binary_writer_t* writer, const anim::AnimResource& value);
};
} // namespace binary

namespace resource sreflect
{
class SKR_ANIM_API SAnimFactory : public SResourceFactory
{
public:
    virtual ~SAnimFactory() noexcept = default;
    skr_guid_t GetResourceType() override;
    bool       AsyncIO() override { return true; }
};
} // namespace resource sreflect
} // namespace skr sreflect