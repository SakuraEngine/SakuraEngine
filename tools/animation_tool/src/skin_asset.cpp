#include "SkrAnimTool/skin_asset.h"
#include "SkrGLTFTool/gltf_utils.hpp"
#include "SkrAnim/resources/skin_resource.h"
#include "cgltf/cgltf.h"

namespace skd::asset
{
bool SSkinCooker::Cook(SCookContext* ctx)
{
    skr_skin_resource_t resource;
    cgltf_data* rawMesh = ctx->Import<cgltf_data>();
    SKR_DEFER({ctx->Destroy(rawMesh);});
    cgltf_skin* rawSkin = &rawMesh->skins[0];
    if (!rawSkin)
    {
        return false;
    }
    auto bin = skr::make_blob_builder<skr_skin_bin_t>();
    bin.joint_remaps.reserve(rawSkin->joints_count);
    for (auto i = 0; i < rawSkin->joints_count; ++i)
        bin.joint_remaps.push_back(rawSkin->joints[i]->name);
    bin.inverse_bind_poses.reserve(rawSkin->joints_count);
    auto matrix = (cgltf_float*)rawSkin->inverse_bind_matrices->buffer_view->data;
    auto components = cgltf_num_components(cgltf_type_mat4);
    SKR_ASSERT(components == 16);
    for (auto i = 0; i < rawSkin->joints_count; ++i)
    {
        bin.inverse_bind_poses.push_back(skr_float4x4_t{ { { matrix[0], matrix[1], matrix[2], matrix[3] },
        { matrix[4], matrix[5], matrix[6], matrix[7] },
        { matrix[8], matrix[9], matrix[10], matrix[11] },
        { matrix[12], matrix[13], matrix[14], matrix[15] } } });
        matrix += components;
    }
    skr_blob_arena_builder_t builder(32);
    skr::binary::BlobHelper<skr_skin_bin_t>::BuildArena(builder, resource.bin, bin);
    resource.arena = builder.build();
    return true;
}
} // namespace skd::asset