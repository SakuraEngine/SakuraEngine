#include "SkrToolCore/asset/cook_system.hpp"
#include "platform/guid.hpp"
#include "utils/defer.hpp"
#include "utils/log.hpp"
#include "SkrGLTFTool/mesh_asset.hpp"
#include "SkrGLTFTool/gltf_utils.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrToolCore/asset/json_utils.hpp"

#include "MeshOpt/meshoptimizer.h"

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

    //----- optimize mesh
    // vertex cache optimization should go first as it provides starting order for overdraw
	const float kOverDrawThreshold = 1.01f; // allow up to 1% worse ACMR to get more reordering opportunities for overdraw
    for (auto& prim : mesh.primitives)
    {
        auto& indices_blob = blobs[prim.index_buffer.buffer_index];
        const auto first_index = prim.index_buffer.first_index;
        const auto index_stride = prim.index_buffer.stride;
        const auto index_offset = prim.index_buffer.index_offset;
        const auto index_count = prim.index_buffer.index_count;
        const auto vertex_count = prim.vertex_count;
        eastl::vector<unsigned int> optimized_indices;
        optimized_indices.resize(index_count);
        unsigned int* indices_ptr = optimized_indices.data();
        for (size_t i = 0; i < index_count; i++)
        {
            if (index_stride == sizeof(uint8_t))
                optimized_indices[i] = *(uint8_t*)(indices_blob.data() + index_offset + (first_index + i) * index_stride);
            else if (index_stride == sizeof(uint16_t))
                optimized_indices[i] = *(uint16_t*)(indices_blob.data() + index_offset + (first_index + i) * index_stride);
            else if (index_stride == sizeof(uint32_t))
                optimized_indices[i] = *(uint32_t*)(indices_blob.data() + index_offset + (first_index + i) * index_stride);
            else if (index_stride == sizeof(uint64_t))
                optimized_indices[i] = *(uint64_t*)(indices_blob.data() + index_offset + (first_index + i) * index_stride);
        }
        for (size_t i = 0; i < index_count; i++)
        {
            SKR_ASSERT(optimized_indices[i] < vertex_count && "Invalid index");
        }
        meshopt_optimizeVertexCache(indices_ptr, indices_ptr, index_count, vertex_count);
        for (const auto& vb : prim.vertex_buffers)
        {
            if (vb.attribute == ESkrVertexAttribute::SKR_VERT_ATTRIB_POSITION)
            {
                auto& vertices_blob = blobs[vb.buffer_index];
                const auto vertex_stride = vb.stride;(void)vertex_stride;
                eastl::vector<skr_float3_t> vertices;
                vertices.resize(vertex_count);
                for (size_t i = 0; i < vertex_count; i++)
                {
                    vertices[i] = *(skr_float3_t*)(vertices_blob.data() + vb.offset);
                }
                meshopt_optimizeOverdraw(indices_ptr, indices_ptr, index_count, 
                    (float*)vertices.data(), vertices.size(), 
                    sizeof(skr_float3_t), kOverDrawThreshold);
                // TODO: meshopt_optimizeVertexFetch
            }
        }
        for (size_t i = 0; i < index_count; i++)
        {
            if (index_stride == sizeof(uint8_t))
                *(uint8_t*)(indices_blob.data() + index_offset + first_index + i * index_stride) = optimized_indices[i];
            else if (index_stride == sizeof(uint16_t))
                *(uint16_t*)(indices_blob.data() + index_offset + first_index + i * index_stride) = optimized_indices[i];
            else if (index_stride == sizeof(uint32_t))
                *(uint32_t*)(indices_blob.data() + index_offset + first_index + i * index_stride) = optimized_indices[i];
            else if (index_stride == sizeof(uint64_t))
                *(uint64_t*)(indices_blob.data() + index_offset + first_index + i * index_stride) = optimized_indices[i];
        }
    }

    //----- write materials
    mesh.materials.reserve(importer->materials.size());
    for (const auto material : importer->materials)
    {
        ctx->AddRuntimeDependency(material);
        mesh.materials.emplace_back(material);
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