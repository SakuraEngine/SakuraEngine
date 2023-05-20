#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrAnimTool/skeleton_asset.h"
#include "SkrToolCore/project/project.hpp"
#include "gltf2ozz.h"

namespace skd::asset
{
void SSkelGltfImporter::Destroy(void* data)
{
    SkrDelete((RawSkeleton*)data);
}

void* SSkelGltfImporter::Import(skr_io_ram_service_t*, SCookContext* context)
{
    using namespace ozz::animation::offline;
    GltfImporter impl;
    ozz::animation::offline::OzzImporter& impoter = impl;
    OzzImporter::NodeType types = {};
    types.skeleton = true;
    auto path = context->AddFileDependency(assetPath.c_str());
    auto fullAssetPath = context->GetAssetRecord()->project->GetAssetPath() / path;
    if(!impoter.Load(fullAssetPath.string().c_str()))
    {
        SKR_LOG_ERROR("Failed to load gltf file %s for asset %s.", assetPath.c_str(), context->GetAssetPath().c_str());
        return nullptr;
    }
    RawSkeleton* rawSkeleton = SkrNew<RawSkeleton>();
    impoter.Import(rawSkeleton, types);
    return rawSkeleton;
}
} // namespace skd::asset