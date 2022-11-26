#include "platform/guid.hpp"
#include "utils/make_zeroed.hpp"
#include "utils/defer.hpp"
#include "utils/log.hpp"
#include "cgpu/api.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrGLTFTool/mesh_asset.hpp"
#include "SkrGLTFTool/gltf_utils.hpp"

namespace 
{

}

void* skd::asset::SGltfMeshImporter::Import(skr::io::RAMService* ioService, SCookContext* context) 
{
    return ImportGLTFWithData(assetPath, ioService, context);
}

void skd::asset::SGltfMeshImporter::Destroy(void* resource)
{
    cgltf_free((cgltf_data*)resource);
}

bool skd::asset::SMeshCooker::Cook(SCookContext* ctx)
{ 
    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    auto cfg = ctx->Config<SMeshCookConfig>();
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
    {
        skr_guid_t shuffle_layout_id = cfg.vertexType;
        CGPUVertexLayout shuffle_layout = {};
        const char* shuffle_layout_name = nullptr;
        if (!shuffle_layout_id.isZero()) 
        {
            shuffle_layout_name = skr_mesh_resource_query_vertex_layout(shuffle_layout_id, &shuffle_layout);
        }

        mesh.name = gltf_data->meshes[0].name;
        if (mesh.name.empty()) mesh.name = "gltfMesh";
        // record buffer bins
        mesh.bins.resize(gltf_data->buffers_count);
        for (uint32_t i = 0; i < gltf_data->buffers_count; i++)
        {
            mesh.bins[i].index = i;
            mesh.bins[i].byte_length = gltf_data->buffers[i].size;
            mesh.bins[i].uri = gltf_data->buffers[i].uri;
            mesh.bins[i].used_with_index = false;
            mesh.bins[i].used_with_vertex = false;

            // transient
            mesh.bins[i].bin.bytes = (uint8_t*)gltf_data->buffers[i].data;
            mesh.bins[i].bin.size = gltf_data->buffers[i].size;
        }
        // record primitvies
        for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
        {
            const auto node_ = gltf_data->nodes + i;
            auto& mesh_section = mesh.sections.emplace_back();
            mesh_section.parent_index = node_->parent ? (int32_t)(node_->parent - gltf_data->nodes) : -1;
            if (node_->has_translation)
                mesh_section.translation = { node_->translation[0], node_->translation[1], node_->translation[2] };
            if (node_->has_scale)
                mesh_section.scale = { node_->scale[0], node_->scale[1], node_->scale[2] };
            if (node_->has_rotation)
                mesh_section.rotation = { node_->rotation[0], node_->rotation[1], node_->rotation[2], node_->rotation[3] };
            if (node_->mesh != nullptr)
            {
                // per primitive
                for (uint32_t j = 0, index_cursor = 0; j < node_->mesh->primitives_count; j++)
                {
                    auto& prim = mesh.primitives.emplace_back();
                    const auto primitive_ = node_->mesh->primitives + j;
                    // ib
                    prim.index_buffer.buffer_index = (uint32_t)(primitive_->indices->buffer_view->buffer - gltf_data->buffers);
                    prim.index_buffer.index_offset = (uint32_t)(primitive_->indices->offset + primitive_->indices->buffer_view->offset);
                    prim.index_buffer.first_index = index_cursor;
                    prim.index_buffer.index_count = (uint32_t)primitive_->indices->count;
                    prim.index_buffer.stride = (uint32_t)primitive_->indices->stride;
                    
                    // vbs
                    prim.vertex_buffers.resize(primitive_->attributes_count);
                    for (uint32_t k = 0, attrib_idx = 0; k < primitive_->attributes_count; k++)
                    {
                        // do shuffle
                        if (shuffle_layout_name != nullptr)
                        {
                            attrib_idx = UINT32_MAX;
                            for (uint32_t l = 0; l < primitive_->attributes_count; l++)
                            {
                                const auto& shuffle_attrib = shuffle_layout.attributes[k];
                                const char* semantic_name = kGLTFAttributeTypeLUT[primitive_->attributes[l].type];
                                if (::strcmp(shuffle_attrib.semantic_name, semantic_name) == 0)
                                {
                                    attrib_idx = l;
                                }
                            }
                            if (attrib_idx == UINT32_MAX) continue; // skip unknwon attribute
                        }
                        else
                        {
                            attrib_idx = k;
                        }
                        const auto buf_view = primitive_->attributes[attrib_idx].data->buffer_view;
                        prim.vertex_buffers[k].buffer_index = (uint32_t)(buf_view->buffer - gltf_data->buffers);
                        prim.vertex_buffers[k].stride = (uint32_t)primitive_->attributes[attrib_idx].data->stride;
                        prim.vertex_buffers[k].offset = (uint32_t)(primitive_->attributes[attrib_idx].data->offset + buf_view->offset);
                    }
                    if (shuffle_layout_name != nullptr)
                    {
                        prim.vertex_layout_id = shuffle_layout_id;
                    }
                    else
                    {
                        SKR_UNREACHABLE_CODE();
                    }

                    // TODO: VertexLayout & Material assignment
                    prim.vertex_layout_id = make_zeroed<skr_guid_t>();
                    prim.material_inst = make_zeroed<skr_guid_t>();

                    mesh_section.primive_indices.emplace_back(mesh.primitives.size() - 1);
                }
            }
        }
    }
    //----- write resource object
    if(!ctx->Save(mesh))
        return false;
    // write bins
    for (auto i = 0; i < gltf_data->buffers_count; i++)
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
        fwrite(gltf_data->buffers[i].data, 1, gltf_data->buffers[i].size, buffer_file);
    }
    return true;
}

uint32_t skd::asset::SMeshCooker::Version() 
{ 
    return kDevelopmentVersion; 
}