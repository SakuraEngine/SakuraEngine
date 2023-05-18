#pragma once
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrAnim/resources/skeleton_resource.h"
#ifndef __meta__
    #include "SkrAnim/resources/skin_resource.generated.h"
#endif

sreflect_struct("guid": "C387FD0E-83BE-4617-9A79-589862F3F941") 
sattr("blob" : true)
skr_skin_blob_view_t
{
    skr::string_view name;
    skr::span<skr::string_view> joint_remaps;
    skr::span<skr_float4x4_t> inverse_bind_poses;
};
    
GENERATED_BLOB_BUILDER(skr_skin_blob_view_t)

sreflect_struct("guid" : "332C6133-7222-4B88-9B2F-E4336A46DF2C")
sattr("rtti" : true)
sattr("serialize" : "bin")
skr_skin_resource_t
{
    spush_attr("no-rtti" : true)
    skr_blob_arena_t arena;
    spush_attr("arena" : "arena")
    skr_skin_blob_view_t blob;
};

namespace skr::resource
{
struct SKR_ANIM_API SSkinFactory : public SResourceFactory 
{
    virtual ~SSkinFactory() = default;
    skr_type_id_t GetResourceType() override;
    bool AsyncIO() override { return true; }
    float AsyncSerdeLoadFactor() override { return 1.0f; }
};
} // namespace skr::resource