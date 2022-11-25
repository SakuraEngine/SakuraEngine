#pragma once
#include "SkrAnim/module.configure.h"
#include "platform/configure.h"
#ifndef __meta__
    #include "SkrAnim/resources/animation_resource.generated.h"
#endif

typedef struct skr_anim_resource_t skr_anim_resource_t;

#ifdef __cplusplus
    #include "SkrAnim/ozz/animation.h"
    #include "binary/reader_fwd.h"
    #include "platform/debug.h"

sreflect_struct("guid": "5D6DC46B-8696-4DD8-ADE4-C27D07CEDCCD")
sattr("rtti" : true)
skr_anim_resource_t
{
    sattr("no-rtti" : true)
    ozz::animation::Animation animation;
};
namespace skr::binary
{
template <>
struct SKR_ANIM_API ReadHelper<skr_anim_resource_t> {
    static int Read(skr_binary_reader_t* reader, skr_anim_resource_t& value);
};
} // namespace skr::binary

    #include "resource/resource_factory.h"
namespace skr sreflect
{
namespace resource sreflect
{
class SAnimFactory : public SResourceFactory
{
public:
    ~SAnimFactory() noexcept = default;
    skr_type_id_t GetResourceType() override;
    bool AsyncIO() override { return true; }
    bool Unload(skr_resource_record_t* record) override;
    ESkrInstallStatus Install(skr_resource_record_t* record) override;
    bool Uninstall(skr_resource_record_t* record) override;
    ESkrInstallStatus UpdateInstall(skr_resource_record_t* record) override;
    void DestroyResource(skr_resource_record_t* record) override;
};
} // namespace resource sreflect
} // namespace skr sreflect
#endif