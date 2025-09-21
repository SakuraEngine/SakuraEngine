#include "cgltf/cgltf.h"
#include "SkrBase/misc/defer.hpp"
#include "SkrCore/log.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrContainers/stl_vector.hpp"
#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrMeshTool/mesh_asset.hpp"
#include "SkrMeshTool/mesh_processing.hpp"
#include "MeshOpt/meshoptimizer.h"

#include "SkrProfile/profile.h"
#include "SkrRTTR/type.hpp"

namespace skd::asset
{
bool MeshCooker::Cook(CookContext* ctx)
{
    auto assetMetaFile = ctx->GetAssetMetaFile();

    auto& mesh_asset = *assetMetaFile->GetMetadata<MeshAsset>();
    if (mesh_asset.vertexType.is_zero())
    {
        SKR_LOG_FATAL(u8"MeshCooker: VertexType is not specified for asset %s!", ctx->GetAssetPath().c_str());
        return false;
    }

    MeshResource mesh;
    mesh.install_to_ram = mesh_asset.install_to_ram;
    mesh.install_to_vram = mesh_asset.install_to_vram;
    //----- write materials
    mesh.materials.reserve(mesh_asset.materials.size());

    for (const auto material : mesh_asset.materials)
    {
        ctx->AddRuntimeDependency(material);
        mesh.materials.add(material);
    }

    const auto importer_type = ctx->GetImporterType();
    skr::Vector<skr::Vector<uint8_t>> blobs;
    if (importer_type == skr::type_id_of<GltfMeshImporter>())
    {
        auto importer = static_cast<GltfMeshImporter*>(ctx->GetImporter());
        auto import_data = ctx->Import<GltfMeshImporter::ImportData>();
        if (!import_data)
        {
            return false;
        }
        SKR_DEFER({ ctx->Destroy(import_data); });

        for (auto import_material : import_data->import_materials)
        {
            ctx->AddRuntimeDependency(import_material);
            mesh.materials.add(import_material);
        }

        if (importer->invariant_vertices)
        {
            CookGLTFMeshData(import_data->gltf_data, &mesh_asset, mesh, blobs);
            // TODO: support ram-only mode install
            mesh.install_to_vram = true;
        }
        else
        {
            CookGLTFMeshData_SplitSkin(import_data->gltf_data, &mesh_asset, mesh, blobs);
            // TODO: install only pos/norm/tangent vertices
            mesh.install_to_ram = true;
            // TODO: support ram-only mode install
            mesh.install_to_vram = true;
        }
    }
    else if (importer_type == skr::type_id_of<ProceduralMeshImporter>())
    {
        auto pBuiltInMesh = ctx->Import<ProceduralMesh>();
        if (!pBuiltInMesh)
        {
            SKR_LOG_FATAL(u8"MeshCooker: Failed to import built-in mesh for asset %s!", ctx->GetAssetPath().c_str());
            return false;
        }
        SKR_DEFER({ ctx->Destroy(pBuiltInMesh); });

        GUID shuffle_layout_id = mesh_asset.vertexType;
        pBuiltInMesh->generate_resource(mesh, blobs, shuffle_layout_id);
    }
    else
    {
        SKR_LOG_FATAL(u8"MeshCooker: Unsupported importer type for asset %s!", ctx->GetAssetPath().c_str());
        return false;
    }

    //----- optimize mesh
    {
        SkrZoneScopedN("WaitOptimizeMesh");

        // allow up to 1% worse ACMR to get more reordering opportunities for overdraw
        const float kOverDrawThreshold = 1.01f;
        // for (auto& prim : mesh.primitives)
        skr::parallel_for(mesh.primitives.begin(), mesh.primitives.end(), 1, [&](auto begin, auto end) {
            SkrZoneScopedN("OptimizeMesh");

            const auto& prim = *begin;
            auto& indices_blob = blobs[prim.index_buffer.buffer_index];
            const auto first_index = prim.index_buffer.first_index;
            const auto index_stride = prim.index_buffer.stride;
            const auto index_offset = prim.index_buffer.index_offset;
            const auto index_count = prim.index_buffer.index_count;
            const auto vertex_count = prim.vertex_count;
            skr::stl_vector<uint64_t> optimized_indices;
            optimized_indices.resize(index_count);
            uint64_t* indices_ptr = optimized_indices.data();
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

            // vertex cache optimization should go first as it provides starting order for overdraw
            meshopt_optimizeVertexCache(indices_ptr, indices_ptr, index_count, vertex_count);

            // reorder indices for overdraw, balancing overdraw and vertex cache efficiency
            for (const auto& vb : prim.vertex_buffers)
            {
                if (vb.attribute == EVertexAttribute::POSITION)
                {
                    auto& vertices_blob = blobs[vb.buffer_index];
                    const auto vertex_stride = vb.stride;
                    (void)vertex_stride;
                    skr::stl_vector<float3> vertices;
                    vertices.resize(vertex_count);
                    for (size_t i = 0; i < vertex_count; i++)
                    {
                        vertices[i] = *(float3*)(vertices_blob.data() + vb.offset);
                    }
                    meshopt_optimizeOverdraw(indices_ptr, indices_ptr, index_count, (float*)vertices.data(), vertices.size(), sizeof(float3), kOverDrawThreshold);

                    // vertex fetch optimization should go last as it depends on the final index order
                    // TODO: meshopt_optimizeVertexFetch
                }
            }

            // write optimized indices
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
        });
    }

    //----- write resource object
    if (!ctx->Save(mesh)) return false;

    // write bins using ResourceVFS
    for (size_t i = 0; i < blobs.size(); i++)
    {
        auto filename = skr::format(u8"{}.buffer{}", assetMetaFile->GetGUID(), i);
        if (!ctx->SaveExtra(blobs[i], filename.c_str()))
        {
            SKR_LOG_FMT_ERROR(u8"[MeshCooker::Cook] failed to write buffer {} for resource {}!",
                i,
                assetMetaFile->GetGUID());
            return false;
        }
    }
    return true;
}

uint32_t MeshCooker::Version()
{
    return kDevelopmentVersion;
}
} // namespace skd::asset