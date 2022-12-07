#include "SkrToolCore/asset/cook_system.hpp"
#include "platform/guid.hpp"
#include "utils/defer.hpp"
#include "utils/log.hpp"
#include "SkrGLTFTool/mesh_asset.hpp"
#include "SkrGLTFTool/gltf_utils.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrToolCore/asset/json_utils.hpp"

void* skd::asset::SGltfMeshImporter::Import(skr_io_ram_service_t* ioService, SCookContext* context) 
{
    skr::filesystem::path relPath = assetPath.data();
    auto ext = relPath.extension();
    if (ext != ".gltf")
    {
        return nullptr;
    }
    const auto assetRecord = context->GetAssetRecord();
    auto path = context->AddFileDependency(relPath).u8string();
    auto vfs = assetRecord->project->vfs;
    return ImportGLTFWithData(path.c_str(), ioService, vfs);
}

void skd::asset::SGltfMeshImporter::Destroy(void* resource)
{
    cgltf_free((cgltf_data*)resource);
}

bool skd::asset::SMeshCooker::Cook(SCookContext* ctx)
{ 
    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    auto cfg = LoadConfig<SMeshCookConfig>(ctx);
    if(cfg.vertexType == skr_guid_t{})
    {
        SKR_LOG_ERROR("MeshCooker: VertexType is not specified for asset %s!", ctx->GetAssetPath().c_str());
        return false;
    }
    auto gltf_data = ctx->Import<cgltf_data>();
    if(!gltf_data)
    {
        return false;
    }
    SKR_DEFER({ ctx->Destroy(gltf_data); });
    skr_mesh_resource_t mesh;
    eastl::vector<eastl::vector<uint8_t>> blobs;
    auto importer = static_cast<SGltfMeshImporter*>(ctx->GetImporter());
    mesh.install_to_ram = importer->install_to_ram;
    mesh.install_to_vram = importer->install_to_vram;
    if (importer->invariant_vertices)
    {
        CookGLTFMeshData(gltf_data, &cfg, mesh, blobs);
        // TODO: support ram-only mode install
        mesh.install_to_vram = true;
    }
    else
    {
        CookGLTFMeshData_SplitSkin(gltf_data, &cfg, mesh, blobs);
        // TODO: install only pos/norm/tangent vertices
        mesh.install_to_ram = true;
        // TODO: support ram-only mode install
        mesh.install_to_vram = true;
    }

    //----- write resource object
    if(!ctx->Save(mesh)) return false;
    // write bins
    for (size_t i = 0; i < blobs.size(); i++)
    {
        auto binOutputPath = outputPath;
        binOutputPath.replace_extension("buffer" + std::to_string(i));
        auto buffer_file = fopen(binOutputPath.u8string().c_str(), "wb");
        SKR_DEFER({ fclose(buffer_file); });
        if (!buffer_file)
        {
            SKR_LOG_FMT_ERROR("[SMeshCooker::Cook] failed to write cooked file for resource {}! path: {}", 
                assetRecord->guid, assetRecord->path.u8string());
            return false;
        }
        fwrite(blobs[i].data(), 1, blobs[i].size(), buffer_file);
    }
    return true;
}

uint32_t skd::asset::SMeshCooker::Version() 
{ 
    return kDevelopmentVersion; 
}