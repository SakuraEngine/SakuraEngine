#pragma once
#include "SkrAnim/module.configure.h"
#include "platform/configure.h"
#ifndef __meta__
    #include "SkrAnim/resources/skeleton_resource.generated.h"
#endif

typedef struct skr_skeleton_resource_t skr_skeleton_resource_t;

#ifdef __cplusplus
    #include "SkrAnim/ozz/skeleton.h"
    #include "binary/reader_fwd.h"
    #include "binary/writer_fwd.h"

sreflect_struct("guid": "1876BF35-E4DC-450B-B9D4-09259397F4BA")
sattr("rtti" : true)
skr_skeleton_resource_t
{
    sattr("no-rtti" : true)
    ozz::animation::Skeleton skeleton;
};

namespace skr::binary
{
template <>
struct SKR_ANIM_API ReadTrait<skr_skeleton_resource_t> {
    static int Read(skr_binary_reader_t* reader, skr_skeleton_resource_t& value);
};
template <>
struct SKR_ANIM_API WriteTrait<const skr_skeleton_resource_t&> {
    static int Write(skr_binary_writer_t* writer, const skr_skeleton_resource_t& value);
};
} // namespace skr::binary

    #include "resource/resource_factory.h"
namespace skr
{
namespace resource
{
struct SKR_ANIM_API SSkelFactory : public SResourceFactory
{
public:
    virtual ~SSkelFactory() noexcept = default;
    skr_type_id_t GetResourceType() override;
    bool AsyncIO() override { return true; }
};
} // namespace resource sreflect
} // namespace skr sreflect
#endif