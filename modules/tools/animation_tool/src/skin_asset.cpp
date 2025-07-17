#include "SkrAnim/resources/skin_resource.hpp"
#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrAnimTool/skin_asset.h"
#include "cgltf/cgltf.h"

namespace skd::asset
{
bool SSkinCooker::Cook(SCookContext* ctx)
{
    using namespace skr::anim;
    SkrZoneScopedNS("SSkinCooker::Cook", 4);

    cgltf_data* rawMesh = ctx->Import<cgltf_data>();
    if (!rawMesh)
    {
        return false;
    }
    SKR_DEFER({ctx->Destroy(rawMesh);});
    //TODO; indexing skin
    cgltf_skin* rawSkin = &rawMesh->skins[0];
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
} // namespace skd::asset