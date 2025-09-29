#pragma once
#include "SkrToolCore/cook_system/importer.hpp"
#include "SkrToolCore/cook_system/cooker.hpp"
#include "SkrToolCore/cook_system/asset_meta.hpp"

#include "SkrAnimTool/skeleton_asset.generated.h" // IWYU pragma: export

namespace ozz::animation::offline
{
struct RawSkeleton;
struct RawAnimation;
} // namespace ozz::animation::offline

namespace skr
{

using RawSkeleton = ozz::animation::offline::RawSkeleton;

struct [[sattr(
    guid = "1719ab02-7a48-45db-b101-949155f92cad" serde = @enable
)]] SKR_ANIMTOOL_API GltfSkelImporter : public skr::Importer
{
    // bool skeleton;
    // bool marker;
    // bool camera;
    // bool light;
    // bool null;
    // bool any;
    skr::String assetPath;
    virtual ~GltfSkelImporter() = default;
    virtual void* Import(skr::io::IRAMService*, CookContext* context) override;
    virtual void Destroy(void*) override;
    static uint32_t Version() { return kDevelopmentVersion; }
};

struct [[sattr(guid = "0198a872-01db-74ca-9382-8f1df4026ca0" serde = @enable)]]
SkeletonAsset : public skr::AssetMetadata
{
    int placeholder; // for future use
};

struct [[sattr(
    guid = "E3581419-8B44-4EF9-89FA-552DA6FE982A"
)]] SKR_ANIMTOOL_API SkelCooker final : public Cooker
{
    bool Cook(CookContext* ctx) override;
    uint32_t Version() override { return kDevelopmentVersion; }
};
} // namespace skr