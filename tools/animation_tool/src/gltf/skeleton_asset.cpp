#include "SkrAnimTool/skeleton_asset.h"
#include "gltf2ozz.h"

namespace skd::asset
{
void SSkelGltfImporter::Destroy(void* data)
{
    SkrDelete((RawSkeleton*)data);
}

void* SSkelGltfImporter::Import(skr::io::RAMService*, SCookContext* context)
{
    using namespace ozz::animation::offline;
    GltfImporter impl;
    ozz::animation::offline::OzzImporter& impoter = impl;
    OzzImporter::NodeType types = {}; // TODO: Fill options
    types.any = any;
    types.camera = camera;
    types.geometry = geometry;
    types.light = light;
    types.marker = marker;
    types.null = null;
    types.skeleton = skeleton;
    RawSkeleton* rawSkeleton = SkrNew<RawSkeleton>();
    impoter.Import(rawSkeleton, types);
    return rawSkeleton;
}
} // namespace skd::asset