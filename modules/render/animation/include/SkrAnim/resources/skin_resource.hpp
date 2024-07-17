#pragma once
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrContainers/span.hpp"
#ifndef __meta__
    #include "SkrAnim/resources/skin_resource.generated.h" // IWYU pragma: export
#endif

namespace skr::anim
{

sreflect_struct("guid" : "332C6133-7222-4B88-9B2F-E4336A46DF2C")
sattr("serde" : "bin")
SkinResource {
    skr::SerializeConstString                            name;
    skr::SerializeConstVector<skr::SerializeConstString> joint_remaps;
    skr::SerializeConstVector<skr_float4x4_t>            inverse_bind_poses;
};

} // namespace skr::anim

namespace skr::resource
{
struct SKR_ANIM_API SSkinFactory : public SResourceFactory {
    virtual ~SSkinFactory() = default;
    skr_guid_t GetResourceType() override;
    bool       AsyncIO() override { return true; }
    float      AsyncSerdeLoadFactor() override { return 1.0f; }
};
} // namespace skr::resource