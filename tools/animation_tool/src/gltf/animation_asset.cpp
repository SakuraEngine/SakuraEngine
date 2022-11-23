#include "SkrAnimTool/animation_asset.h"
#include "gltf2ozz.h"

namespace skd::asset
{
void SAnimGltfImporter::Destroy(void* data)
{
    SkrDelete((RawAnimation*)data);
}

void* SAnimGltfImporter::Import(skr::io::RAMService*, SCookContext* context)
{
    GltfImporter impl;
    ozz::animation::offline::OzzImporter& impoter = impl;
    auto& skeletonResource = context->GetStaticDependency(0);
    ozz::animation::Skeleton& skeleton = *(ozz::animation::Skeleton*)skeletonResource.get_ptr();
    RawAnimation* rawAnimation = SkrNew<RawAnimation>();
    impoter.Import(animationName.c_str(), skeleton,
                        samplingRate, rawAnimation);
    return rawAnimation;
}
} // namespace skd::asset