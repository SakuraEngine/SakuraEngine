#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrAnimTool/animation_asset.h"
#include "SkrToolCore/project/project.hpp"
#include "gltf2ozz.h"
#include "SkrAnim/ozz/base/memory/allocator.h"

namespace skr
{
void GltfAnimImporter::Destroy(void* data)
{
    RawAnimation* raw = (RawAnimation*)data;
    raw->~RawAnimation();
    ozz::memory::default_allocator()->Deallocate(data, alignof(RawAnimation));
}

void* GltfAnimImporter::Import(skr::io::IRAMService*, CookContext* context)
{
    GltfOzzImporter impl;
    ozz::animation::offline::OzzImporter& impoter = impl;
    auto& skeletonResource = context->GetStaticDependency(0);
    ozz::animation::Skeleton& skeleton = *(ozz::animation::Skeleton*)skeletonResource.get_loaded();
    auto path = context->AddSourceFile(assetPath.c_str());
    auto fullAssetPath = context->GetAssetMetaFile()->GetProject()->GetAssetPath() / path;
    if (!impoter.Load(reinterpret_cast<const char*>(fullAssetPath.string().c_str())))
    {
        SKR_LOG_ERROR(u8"Failed to load gltf file %s for asset %s.", assetPath.c_str(), context->GetAssetPath().c_str());
        return nullptr;
    }
    RawAnimation* rawAnimation = SkrNew<RawAnimation>();
    impoter.Import(animationName.c_str_raw(), skeleton, samplingRate, rawAnimation);
    return rawAnimation;
}
} // namespace skr