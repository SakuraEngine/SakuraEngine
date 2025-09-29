#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrAnimTool/skeleton_asset.h"
#include "SkrToolCore/project/project.hpp"
#include "gltf2ozz.h"
#include "SkrAnim/ozz/base/memory/allocator.h"

namespace skr
{
void GltfSkelImporter::Destroy(void* data)
{
    RawSkeleton* raw = (RawSkeleton*)data;
    raw->~RawSkeleton();
    ozz::memory::default_allocator()->Deallocate(data, alignof(RawSkeleton));
}

void* GltfSkelImporter::Import(skr::io::IRAMService*, CookContext* context)
{
    using namespace ozz::animation::offline;
    GltfOzzImporter impl;
    ozz::animation::offline::OzzImporter& impoter = impl;
    OzzImporter::NodeType types = {};
    types.skeleton = true;
    auto path = context->AddSourceFile(assetPath.c_str());
    auto fullAssetPath = context->GetAssetMetaFile()->GetProject()->GetAssetPath() / path;
    if (!impoter.Load(reinterpret_cast<const char*>(fullAssetPath.string().c_str())))
    {
        SKR_LOG_ERROR(u8"Failed to load gltf file %s for asset %s.", assetPath.c_str(), context->GetAssetPath().c_str());
        return nullptr;
    }
    RawSkeleton* rawSkeleton = SkrNew<RawSkeleton>();
    impoter.Import(rawSkeleton, types);
    return rawSkeleton;
}
} // namespace skr