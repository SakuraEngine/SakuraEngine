#include "ecs/dual.h"
#include "utils/io.hpp"
#include "mesh_asset.hpp"
#include "skr_renderer/resources/mesh_resource.h"
#include "resource/resource_factory.h"
#include "utils/make_zeroed.hpp"
#include "utils/defer.hpp"
#include "utils/log.hpp"
#include "platform/guid.hpp"

#define MAGIC_SIZE_GLTF_PARSE_READY ~0

void* skd::asset::SGltfMeshImporter::Import(skr::io::RAMService* ioService, SCookContext* context) 
{
    skr::filesystem::path relPath = assetPath.c_str();
    const auto assetRecord = context->GetAssetRecord();
    auto ext = relPath.extension();
    if (ext != ".gltf")
    {
        return nullptr;
    }
    auto path = context->AddFileDependency(relPath);
    // prepare callback
    auto u8Path = path.u8string();
    skr::task::event_t counter;
    struct CallbackData
    {
        skr_async_ram_destination_t destination;
        skr::task::event_t* pCounter;   
        eastl::string u8Path;
    } callbackData;
    callbackData.pCounter = &counter;
    callbackData.u8Path = u8Path.c_str();
    // prepare io
    skr_ram_io_t ramIO = {};
    ramIO.offset = 0;
    ramIO.path = u8Path.c_str();
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
        auto cbData = (CallbackData*)data;
        cgltf_options options = {};
        struct cgltf_data* gltf_data_ = nullptr;
        if (cbData->destination.bytes)
        {
            cgltf_result result = cgltf_parse(&options, cbData->destination.bytes, cbData->destination.size, &gltf_data_);
            if (result != cgltf_result_success)
            {
                gltf_data_ = nullptr;
            }
            else
            {
                result = cgltf_load_buffers(&options, gltf_data_, cbData->u8Path.c_str());
                result = cgltf_validate(gltf_data_);
                if (result != cgltf_result_success)
                {
                    gltf_data_ = nullptr;
                }
            }
        }
        sakura_free(cbData->destination.bytes);
        cbData->destination.bytes = (uint8_t*)gltf_data_;
        cbData->destination.size = MAGIC_SIZE_GLTF_PARSE_READY;
        cbData->pCounter->signal();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&callbackData;
    skr_async_io_request_t ioRequest = {};
    ioService->request(assetRecord->project->vfs, &ramIO, &ioRequest, &callbackData.destination);
    counter.wait(false);
    // parse
    if (callbackData.destination.size == MAGIC_SIZE_GLTF_PARSE_READY)
    {
        cgltf_data* gltf_data = (cgltf_data*)callbackData.destination.bytes;
        auto mesh = SkrNew<skr_mesh_resource_t>();
        mesh->name = gltf_data->meshes[0].name;
        if (mesh->name.empty()) mesh->name = "gltfMesh";
        // record buffer bins
        mesh->bins.resize(gltf_data->buffers_count);
        for (uint32_t i = 0; i < gltf_data->buffers_count; i++)
        {
            mesh->bins[i].bin.bytes = (uint8_t*)gltf_data->buffers[i].data;
            mesh->bins[i].bin.size = gltf_data->buffers[i].size;
            mesh->bins[i].uri = gltf_data->buffers[i].uri;
            mesh->bins[i].used_with_index = false;
            mesh->bins[i].used_with_vertex = false;
        }
        // record primitvies
        for (uint32_t i = 0; i < gltf_data->nodes_count; i++)
        {
            const auto node_ = gltf_data->nodes + i;
            auto& mesh_section = mesh->sections.emplace_back();
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
                    auto& prim = mesh->primitives.emplace_back();
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
                        /*
                        if (shuffle_layout_name != nullptr)
                        {
                            attrib_idx = -1;
                            for (uint32_t l = 0; l < primitive_->attributes_count; l++)
                            {
                                const auto& shuffle_attrib = shuffle_layout.attributes[k];
                                const char* semantic_name = cGLTFAttributeTypeLUT[primitive_->attributes[l].type];
                                if (::strcmp(shuffle_attrib.semantic_name, semantic_name) == 0)
                                {
                                    attrib_idx = l;
                                }
                            }
                        }
                        else
                        */
                        {
                            attrib_idx = k;
                        }
                        const auto buf_view = primitive_->attributes[attrib_idx].data->buffer_view;
                        prim.vertex_buffers[k].buffer_index = (uint32_t)(buf_view->buffer - gltf_data->buffers);
                        prim.vertex_buffers[k].stride = (uint32_t)primitive_->attributes[attrib_idx].data->stride;
                        prim.vertex_buffers[k].offset = (uint32_t)(primitive_->attributes[attrib_idx].data->offset + buf_view->offset);
                    }
                    /*
                    if (shuffle_layout_name != nullptr)
                    {
                        prim.vertex_layout_id = shuffle_layout_id;
                    }
                    else
                    {
                        prim.vertex_layout_id = mesh_resource_util.AddOrFindVertexLayoutFromGLTFPrimitive(primitive_);
                    }
                    */

                    // TODO: VertexLayout & Material assignment
                    prim.vertex_layout_id = make_zeroed<skr_guid_t>();
                    prim.material_inst = make_zeroed<skr_guid_t>();

                    mesh_section.primive_indices.emplace_back(mesh->primitives.size() - 1);
                }
            }
        }
        mesh->gltf_data = gltf_data;
        return mesh;
    }
    return nullptr;
}

void skd::asset::SGltfMeshImporter::Destroy(void* resource)
{
    auto mesh = (skr_mesh_resource_t*)resource;
    if (mesh->gltf_data) cgltf_free((cgltf_data*)mesh->gltf_data);
    SkrDelete(mesh);
}

bool skd::asset::SMeshCooker::Cook(SCookContext* ctx)
{ 
    const auto outputPath = ctx->GetOutputPath();
    const auto assetRecord = ctx->GetAssetRecord();
    auto mesh = ctx->Import<skr_mesh_resource_t>();
    auto gltf_data = (cgltf_data*)mesh->gltf_data;
    if(!mesh)
    {
        return false;
    }
    SKR_DEFER({ ctx->Destroy(mesh); });
    //-----write resource header
    eastl::vector<uint8_t> buffer;
    //TODO: 公共化 VectorWriter
    struct VectorWriter
    {
        eastl::vector<uint8_t>* buffer;
        int write(const void* data, size_t size)
        {
            buffer->insert(buffer->end(), (uint8_t*)data, (uint8_t*)data + size);
            return 0;
        }
    } writer{&buffer};
    skr_binary_writer_t archive(writer);
    //------write resource object
    skr::binary::Archive(&archive, *mesh);
    // archive(resource->sections);
    //------save resource to disk
    auto file = fopen(outputPath.u8string().c_str(), "wb");
    SKR_DEFER({ fclose(file); });
    if (!file)
    {
        SKR_LOG_FMT_ERROR("[SConfigCooker::Cook] failed to write cooked file for resource {}! path: {}", 
            assetRecord->guid, assetRecord->path.u8string());
        return false;
    }
    fwrite(buffer.data(), 1, buffer.size(), file);
    // write bins
    for (auto i = 0; i < gltf_data->buffers_count; i++)
    {
        auto binOutputPath = outputPath;
        binOutputPath.replace_extension("buffer" + std::to_string(i));
        auto buffer_file = fopen(binOutputPath.u8string().c_str(), "wb");
        SKR_DEFER({ fclose(buffer_file); });
        if (!buffer_file)
        {
            SKR_LOG_FMT_ERROR("[SConfigCooker::Cook] failed to write cooked file for resource {}! path: {}", 
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