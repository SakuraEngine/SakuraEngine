#include "SkrAnim/resources/skin_resource.hpp"
#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrAnimTool/skin_asset.h"
#include "cgltf/cgltf.h"
#include "SkrMeshTool/mesh_asset.hpp"

namespace skr
{
bool SkinCooker::Cook(CookContext* ctx)
{
    using namespace skr;
    SkrZoneScopedNS("SkinCooker::Cook", 4);

    auto import_data = ctx->Import<GltfMeshImporter::ImportData>();
    if (!import_data)
    {
        return false;
    }
    cgltf_data* rawMesh = import_data->gltf_data;
    if (!rawMesh)
    {
        return false;
    }
    SKR_DEFER({ ctx->Destroy(import_data); });
    // TODO; indexing skin
    cgltf_skin* rawSkin = &rawMesh->skins[0];
    if (!rawSkin)
    {
        SKR_LOG_ERROR(u8"[SkinCooker::Cook] No skin found in gltf file!");
        return false;
    }

    SkinResource resource;
    resource.name = rawSkin->name ? (const char8_t*)rawSkin->name : u8"UnnamedSkin";
    resource.joint_remaps.reserve(rawSkin->joints_count);
    for (auto i = 0; i < rawSkin->joints_count; ++i)
        resource.joint_remaps.add((const char8_t*)rawSkin->joints[i]->name);
    auto buffer_view = rawSkin->inverse_bind_matrices->buffer_view;
    const auto buffer_data = static_cast<const uint8_t*>(buffer_view->data ? buffer_view->data : buffer_view->buffer->data);
    auto matrix = (cgltf_float*)(buffer_data + buffer_view->offset);
    auto components = cgltf_num_components(cgltf_type_mat4);
    SKR_ASSERT(components == 16);
    resource.inverse_bind_poses.resize_default(rawSkin->joints_count);
    std::memcpy((void*)resource.inverse_bind_poses.data(), matrix, sizeof(cgltf_float) * components * rawSkin->joints_count);
    return ctx->Save(resource);
}
} // namespace skr