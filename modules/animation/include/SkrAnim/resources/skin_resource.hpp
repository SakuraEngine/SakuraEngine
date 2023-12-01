#pragma once
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrRT/containers_new/span.hpp"
#ifndef __meta__
    #include "SkrAnim/resources/skin_resource.generated.h" // IWYU pragma: export
#endif

namespace skr sreflect
{
namespace anim sreflect
{

sreflect_struct("guid": "C387FD0E-83BE-4617-9A79-589862F3F941") 
sattr("blob" : true)
SkinBlobView {
    skr::StringView            name;
    skr::span<skr::StringView> joint_remaps;
    skr::span<skr_float4x4_t>   inverse_bind_poses;
};

GENERATED_BLOB_BUILDER(SkinBlobView)

sreflect_struct("guid" : "332C6133-7222-4B88-9B2F-E4336A46DF2C")
sattr("serialize" : "bin")
SkinResource {
    skr_blob_arena_t arena;
    spush_attr("arena" : "arena")
    SkinBlobView     blob;
};

} // namespace anim sreflect
} // namespace skr sreflect

namespace skr::resource
{
struct SKR_ANIM_API SSkinFactory : public SResourceFactory {
    virtual ~SSkinFactory() = default;
    skr_guid_t GetResourceType() override;
    bool       AsyncIO() override { return true; }
    float      AsyncSerdeLoadFactor() override { return 1.0f; }
};
} // namespace skr::resource