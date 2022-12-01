#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrAnimTool/animation_asset.h"
#include "gltf2ozz.h"

namespace skd::asset
{
void SAnimGltfImporter::Destroy(void* data)
{
    SkrDelete((RawAnimation*)data);
}

void* SAnimGltfImporter::Import(skr_io_ram_service_t*, SCookContext* context)
{
    GltfImporter impl;
    ozz::animation::offline::OzzImporter& impoter = impl;
    auto& skeletonResource = context->GetStaticDependency(0);
    ozz::animation::Skeleton& skeleton = *(ozz::animation::Skeleton*)skeletonResource.get_ptr();
    if(!impoter.Load(context->AddFileDependency(assetPath.c_str()).u8string().c_str()))
    {
        SKR_LOG_FMT_ERROR("Failed to load gltf file %s for asset %s.", assetPath.c_str(), context->GetAssetPath());
        return nullptr;
    }
    RawAnimation* rawAnimation = SkrNew<RawAnimation>();
    impoter.Import(animationName.c_str(), skeleton,
                        samplingRate, rawAnimation);
    return rawAnimation;
}
} // namespace skd::asset