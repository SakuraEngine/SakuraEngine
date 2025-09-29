#pragma once
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrContainers/span.hpp"
#include "SkrAnim/resources/skin_resource.generated.h" // IWYU pragma: export

namespace skr
{

struct [[sattr(
    guid = "332C6133-7222-4B88-9B2F-E4336A46DF2C"
    serde = @enable
)]] SkinResource
{
    skr::SerializeConstString name;
    skr::SerializeConstVector<skr::SerializeConstString> joint_remaps;
    skr::SerializeConstVector<skr::float4x4> inverse_bind_poses;
};

struct SKR_ANIM_API SkinFactory : public ResourceFactory
{
    virtual ~SkinFactory() = default;
    GUID GetResourceType() override;
    bool AsyncIO() override { return true; }
    float AsyncSerdeLoadFactor() override { return 1.0f; }
};
} // namespace skr