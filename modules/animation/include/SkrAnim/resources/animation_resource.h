#pragma once
#include "SkrAnim/module.configure.h"
#include "SkrRT/platform/configure.h"
#ifndef __meta__
    #include "SkrAnim/resources/animation_resource.generated.h" // IWYU pragma: export
#endif
typedef struct skr_anim_resource_t skr_anim_resource_t;

#ifdef __cplusplus
    #include "SkrRT/resource/resource_factory.h"
    #include "SkrRT/serde/binary/reader_fwd.h"
    #include "SkrRT/serde/binary/writer_fwd.h"
    #include "SkrAnim/ozz/animation.h"

sreflect_struct("guid": "5D6DC46B-8696-4DD8-ADE4-C27D07CEDCCD")
sattr("rtti" : true)
skr_anim_resource_t {
    sattr("no-rtti" : true)
    ozz::animation::Animation animation;
};
namespace skr::binary
{
template <>
struct SKR_ANIM_API ReadTrait<skr_anim_resource_t> {
    static int Read(skr_binary_reader_t* reader, skr_anim_resource_t& value);
};
template <>
struct SKR_ANIM_API WriteTrait<skr_anim_resource_t> {
    static int Write(skr_binary_writer_t* writer, const skr_anim_resource_t& value);
};
} // namespace skr::binary

namespace skr sreflect
{
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
#endif