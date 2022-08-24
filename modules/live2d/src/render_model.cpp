#include "platform/vfs.h"

#include "utils/io.hpp"
#include "utils/log.h"
#include "utils/defer.hpp"
#include "cgpu/io.hpp"

#include "utils/make_zeroed.hpp"
#include "platform/memory.h"
#include "platform/debug.h"
#include "skr_live2d/render_model.h"
#include "live2d_clipping.hpp"
#include "live2d_helpers.hpp"
#include <ghc/filesystem.hpp>

#include "skr_image_coder/skr_image_coder.h"
#ifdef _WIN32
#include "skr_image_coder/extensions/win_dstorage_decompressor.h"
#endif

#include "tracy/Tracy.hpp"

struct skr_live2d_render_model_impl_t : public skr_live2d_render_model_t {
    virtual ~skr_live2d_render_model_impl_t() SKR_NOEXCEPT
    {
        for (auto&& texture : textures)
        {
            cgpu_free_texture(texture);
        }
        for (auto&& texture_view : texture_views)
        {
            cgpu_free_texture_view(texture_view);
        }
        cgpu_free_buffer(index_buffer);
        cgpu_free_buffer(pos_buffer);
        cgpu_free_buffer(uv_buffer);
        CSM_DELETE_SELF(skr_live2d_clipping_manager_t, clipping_manager);
    }

    CGPUBufferId index_buffer;
    CGPUBufferId pos_buffer;
    CGPUBufferId uv_buffer;

    eastl::vector<skr_async_io_request_t> texture_io_requests;
    eastl::vector<skr_async_vtexture_destination_t> texture_destinations;
    eastl::vector<skr_async_io_request_t> png_io_requests;
    eastl::vector<skr_async_ram_destination_t> png_destinations;

    eastl::vector<skr_async_io_request_t> buffer_io_requests;
    eastl::vector<skr_async_vbuffer_destination_t> buffer_destinations;
};

struct skr_live2d_render_model_async_t : public skr_live2d_render_model_impl_t {
    skr_live2d_render_model_async_t() = delete;
    skr_live2d_render_model_async_t(skr_live2d_render_model_request_t* request, skr_live2d_model_resource_id model_resource)
        : skr_live2d_render_model_impl_t(), request(request)
    {
        model_resource_id = model_resource;
        auto model = model_resource->model->GetModel();
        if (model->IsUsingMasking())
        {
            clipping_manager = CSM_NEW skr_live2d_clipping_manager_t();
            clipping_manager->Initialize(*model, 
                model->GetDrawableCount(),
                model->GetDrawableMasks(), 
                model->GetDrawableMaskCounts()
            );
        }
    }
    void texture_finish(skr_async_io_request_t* p_io_request)
    {
        finished_texture_request++;
        try_finish();
    }
    void buffer_finish(skr_async_io_request_t* p_io_request)
    {
        finished_buffer_request++;
        try_finish();
    }
    void finish()
    {
        for (uint32_t i = 0; i < textures.size(); i++)
        {
            textures[i] = texture_destinations[i].texture;
            CGPUTextureViewDescriptor view_desc = {};
            view_desc.texture = textures[i];
            view_desc.array_layer_count = 1;
            view_desc.base_array_layer = 0;
            view_desc.mip_level_count = 1;
            view_desc.base_mip_level = 0;
            view_desc.aspects = CGPU_TVA_COLOR;
            view_desc.dims = CGPU_TEX_DIMENSION_2D;
            view_desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
            view_desc.usages = CGPU_TVU_SRV;
            texture_views[i] = cgpu_create_texture_view(textures[i]->device, &view_desc);
        }
        for (auto&& coder : coders)
        {
            if (coder) skr_image_coder_free_image(coder);
        }
        skr_atomic32_store_relaxed(&request->io_status, SKR_ASYNC_IO_STATUS_OK);
        request = nullptr;
    }
    void try_finish()
    {
        if (finished_texture_request < texture_io_requests.size()) return;
        const auto bios = buffer_io_requests.size();
        if (finished_buffer_request < bios) return;
        finish();
    }
    uint32_t finished_texture_request = 0;
    uint32_t finished_buffer_request = 0;
    skr_live2d_render_model_request_t* request = nullptr;
    skr_io_vram_service_t* vram_service = nullptr;
    CGPUDeviceId device;
    CGPUQueueId transfer_queue;
    eastl::vector<skr_image_coder_id> coders;
};

bool skr_live2d_render_model_request_t::is_ready() const SKR_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_OK;
}

SkrAsyncIOStatus skr_live2d_render_model_request_t::get_status() const SKR_NOEXCEPT
{
    return (SkrAsyncIOStatus)skr_atomic32_load_acquire(&io_status);
}

#ifndef SKR_SERIALIZE_GURAD
void skr_live2d_render_model_create_from_raw(skr_io_ram_service_t* ram_service, skr_io_vram_service_t* vram_service, 
    CGPUDeviceId device, skr_live2d_model_resource_id resource, skr_live2d_render_model_request_t* request)
{
    auto csmModel = resource->model->GetModel();
    SKR_ASSERT(csmModel && "csmModel is null");
    auto file_dstorage_queue = request->file_dstorage_queue_override;
    auto memory_dstorage_queue = request->memory_dstorage_queue_override;
    const uint32_t texture_count = resource->model_setting->GetTextureCount();
    auto render_model = SkrNew<skr_live2d_render_model_async_t>(request, resource);
    request->render_model = render_model;
    render_model->device = device;
    render_model->transfer_queue = request->queue_override;
    render_model->vram_service = vram_service;
    // request load textures
    render_model->textures.resize(texture_count);
    render_model->texture_views.resize(texture_count);
    render_model->texture_destinations.resize(texture_count);
    render_model->texture_io_requests.resize(texture_count);
    if (!request->file_dstorage_queue_override)
    {
        render_model->png_destinations.resize(texture_count);
        render_model->png_io_requests.resize(texture_count);
    }
    for (uint32_t i = 0; i < texture_count; i++)
    {
        ZoneScopedN("RequestLive2DTexture");

        auto& texture_destination = render_model->texture_destinations[i];
        auto& texture_io_request = render_model->texture_io_requests[i];
        auto vram_texture_io = make_zeroed<skr_vram_texture_io_t>();
        auto texture_path = resource->model_setting->GetTextureFileName(i);
        auto pngPath = ghc::filesystem::path(request->vfs_override->mount_dir) / resource->model->homePath.c_str() / texture_path;
        auto pngPathStr = pngPath.u8string();
        if (request->file_dstorage_queue_override)
        {
            std::string p = texture_path;
            int p1 = p.find(".");
            int p2 = p.find("/");
            std::string number = p.substr(p1 + 1, p2 - p1 - 1);
            auto resolution = std::stoi(number);
            vram_texture_io.device = device;
        
            vram_texture_io.dstorage.path = pngPathStr.c_str();
            vram_texture_io.dstorage.compression = SKR_WIN_DSTORAGE_COMPRESSION_TYPE_IMAGE;
            vram_texture_io.dstorage.source_type = CGPU_DSTORAGE_SOURCE_FILE;
            vram_texture_io.dstorage.queue = file_dstorage_queue;
            vram_texture_io.dstorage.uncompressed_size = resolution * resolution * 4;

            vram_texture_io.vtexture.texture_name = texture_path;
            vram_texture_io.vtexture.resource_types = CGPU_RESOURCE_TYPE_TEXTURE;
            vram_texture_io.vtexture.width = resolution;
            vram_texture_io.vtexture.height = resolution;
            vram_texture_io.vtexture.depth = 1;
            vram_texture_io.vtexture.format = CGPU_FORMAT_R8G8B8A8_UNORM;
            
            vram_texture_io.src_memory.size = resolution * resolution * 4;

            vram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data){
                auto render_model = (skr_live2d_render_model_async_t*)data;
                render_model->texture_finish(request);
            };
            vram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = render_model;
            vram_service->request(&vram_texture_io, &texture_io_request, &texture_destination);
        }
        else
        {
            auto& png_destination = render_model->png_destinations[i];
            auto& png_io_request = render_model->png_io_requests[i];

            auto ram_texture_io = make_zeroed<skr_ram_io_t>();

            ram_texture_io.path = pngPathStr.c_str();
            ram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
                ZoneScopedN("Load PNG");
                auto render_model = (skr_live2d_render_model_async_t*)data;
                auto idx = request - render_model->png_io_requests.data();

                const auto& png_destination = render_model->png_destinations[idx];
                auto vram_service = render_model->vram_service;
                // decompress
                EImageCoderFormat format = skr_image_coder_detect_format((const uint8_t*)png_destination.bytes, png_destination.size);
                auto coder = skr_image_coder_create_image(format);
                render_model->coders.emplace_back(coder);
                if (skr_image_coder_set_encoded(coder, (const uint8_t*)png_destination.bytes, png_destination.size))
                {
                    SKR_LOG_DEBUG("image coder: width = %d, height = %d, encoded_size = %d, raw_size = %d", 
                        skr_image_coder_get_width(coder), skr_image_coder_get_height(coder), 
                        skr_image_coder_get_encoded_size(coder),
                        skr_image_coder_get_raw_size(coder));
                    const auto encoded_format = coder->get_color_format();
                    const auto raw_format = (encoded_format == IMAGE_CODER_COLOR_FORMAT_BGRA) ? IMAGE_CODER_COLOR_FORMAT_RGBA : encoded_format;
                    if (auto raw_data = coder->get_raw_data_view(raw_format, coder->get_bit_depth());!raw_data.empty())
                    {
                        // upload
                        auto& texture_io_request = render_model->texture_io_requests[idx];
                        auto& texture_destination = render_model->texture_destinations[idx];
                        auto vram_texture_io = make_zeroed<skr_vram_texture_io_t>();
                        vram_texture_io.device = render_model->device;
                        vram_texture_io.transfer_queue = render_model->transfer_queue;

                        vram_texture_io.vtexture.texture_name = nullptr;
                        vram_texture_io.vtexture.resource_types = CGPU_RESOURCE_TYPE_TEXTURE;
                        vram_texture_io.vtexture.width = coder->get_width();
                        vram_texture_io.vtexture.height = coder->get_height();
                        vram_texture_io.vtexture.depth = 1;
                        vram_texture_io.vtexture.format = CGPU_FORMAT_R8G8B8A8_UNORM;

                        vram_texture_io.src_memory.size = coder->get_width() * coder->get_height() * 4;
                        vram_texture_io.src_memory.bytes = raw_data.data();
                        vram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data){
                            auto render_model = (skr_live2d_render_model_async_t*)data;

                            render_model->texture_finish(request);
                        };
                        vram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = render_model;
                        vram_service->request(&vram_texture_io, &texture_io_request, &texture_destination);
                    }
                }
                sakura_free(png_destination.bytes);
            };
            ram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)render_model;
            ram_service->request(request->vfs_override, &ram_texture_io, &png_io_request, &png_destination);
        }
    }
    
    // request load buffers
    const auto use_dynamic_buffer = render_model->use_dynamic_buffer;
    const auto drawable_count = csmModel->GetDrawableCount();
    uint32_t total_index_count = 0;
    uint32_t total_vertex_count = 0;
    for(uint32_t i = 0; i < drawable_count; i++)
    {
        const int32_t vcount = csmModel->GetDrawableVertexCount(i);
        total_vertex_count += vcount;
        const int32_t icount = csmModel->GetDrawableVertexIndexCount(i);
        total_index_count += icount;
    }
    // Create Vertex Buffer
    {
        ZoneScopedN("CreateLive2DVertexBuffer");

        auto vb_desc = make_zeroed<CGPUBufferDescriptor>();
        eastl::string name = resource->model_setting->GetModelFileName();
        auto pos_name = name + eastl::string("-pos");
        vb_desc.name = pos_name.c_str();
        vb_desc.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
        vb_desc.flags = use_dynamic_buffer ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE;
        vb_desc.memory_usage = use_dynamic_buffer ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY;
        vb_desc.prefer_on_device = true;
        vb_desc.size = total_vertex_count * sizeof(skr_live2d_vertex_pos_t);
        render_model->pos_buffer = cgpu_create_buffer(device, &vb_desc);

        auto uv_name = name + eastl::string("-uv");
        vb_desc.name = uv_name.c_str();
        vb_desc.size = total_vertex_count * sizeof(skr_live2d_vertex_uv_t);
        render_model->uv_buffer = cgpu_create_buffer(device, &vb_desc);
    }
    // Create Index Buffer
    {
        ZoneScopedN("CreateLive2DIndexBuffer");

        auto ib_desc = make_zeroed<CGPUBufferDescriptor>();
        eastl::string name = resource->model_setting->GetModelFileName();
        auto ind_name = name + eastl::string("-i");
        ib_desc.name = ind_name.c_str();
        ib_desc.descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER;
        ib_desc.flags = CGPU_BCF_NONE;
        ib_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
        ib_desc.size = total_index_count * sizeof(Csm::csmUint16);
        render_model->index_buffer = cgpu_create_buffer(device, &ib_desc);
    }
    // Record Vertex Buffer View
    {
        render_model->vertex_buffer_views.resize(2 * drawable_count);
        uint32_t pos_buffer_cursor = 0;
        uint32_t uv_buffer_cursor = 0;
        for(uint32_t i = 0; i < drawable_count; i++)
        {
            const int32_t vcount = csmModel->GetDrawableVertexCount(i);
            if (vcount != 0)
            {
                ZoneScopedN("FillLive2DVertexBufferViews");
                auto pos_slot = 2 * i;
                auto uv_slot = 2 * i + 1;
                render_model->vertex_buffer_views[pos_slot].offset = pos_buffer_cursor;
                render_model->vertex_buffer_views[pos_slot].stride = sizeof(skr_live2d_vertex_pos_t);
                render_model->vertex_buffer_views[pos_slot].buffer = render_model->pos_buffer;
                pos_buffer_cursor += vcount * sizeof(skr_live2d_vertex_pos_t);

                render_model->vertex_buffer_views[uv_slot].offset = uv_buffer_cursor;
                render_model->vertex_buffer_views[uv_slot].stride = sizeof(skr_live2d_vertex_uv_t);
                render_model->vertex_buffer_views[uv_slot].buffer = render_model->uv_buffer;
                uv_buffer_cursor += vcount * sizeof(skr_live2d_vertex_uv_t);
            }
        }
    }
    // Record Index Buffer View
    {
        render_model->primitive_commands.resize(drawable_count);
        render_model->index_buffer_views.resize(drawable_count);
        uint32_t index_buffer_cursor = 0;
        for(uint32_t i = 0; i < drawable_count; i++)
        {
            const int32_t icount = csmModel->GetDrawableVertexIndexCount(i);
            if (icount != 0)
            {
                render_model->index_buffer_views[i].offset = 0;
                render_model->index_buffer_views[i].first_index = index_buffer_cursor;
                render_model->index_buffer_views[i].index_count = icount;
                render_model->index_buffer_views[i].stride = sizeof(Csm::csmUint16);
                render_model->index_buffer_views[i].buffer = render_model->index_buffer;
                index_buffer_cursor += icount;
            }
            // Record static primitive commands
            render_model->primitive_commands[i].ibv = &render_model->index_buffer_views[i];
            render_model->primitive_commands[i].vbvs = skr::span(&render_model->vertex_buffer_views[2 * i], 2);
        }
    }
    // Request indices I/O
    {
        render_model->buffer_io_requests.resize(drawable_count);
        render_model->buffer_destinations.resize(drawable_count);
        uint32_t index_buffer_cursor = 0;
        for(uint32_t i = 0; i < drawable_count; i++)
        {
            const int32_t icount = csmModel->GetDrawableVertexIndexCount(i);
            if (icount != 0)
            {
                ZoneScopedN("RequestLive2DIndexBuffer");

                const auto indices = csmModel->GetDrawableVertexIndices(i);
                auto ib_io = make_zeroed<skr_vram_buffer_io_t>();
                auto& io_request = render_model->buffer_io_requests[i];
                auto& buffer_destination = render_model->buffer_destinations[i];
                buffer_destination.buffer = render_model->index_buffer;

                ib_io.device = device;
                ib_io.transfer_queue = memory_dstorage_queue ? nullptr : request->queue_override;

                ib_io.dstorage.queue = memory_dstorage_queue;
                ib_io.dstorage.source_type = CGPU_DSTORAGE_SOURCE_MEMORY;
                ib_io.dstorage.uncompressed_size = sizeof(Csm::csmUint16) * icount;

                ib_io.vbuffer.offset = index_buffer_cursor;
                ib_io.vbuffer.resource_types = CGPU_RESOURCE_TYPE_INDEX_BUFFER;
                ib_io.vbuffer.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
                ib_io.vbuffer.buffer_size = total_index_count * sizeof(Csm::csmUint16);
                
                ib_io.src_memory.bytes = (uint8_t*)indices;
                ib_io.src_memory.size = sizeof(Csm::csmUint16) * icount;
                ib_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data){
                    auto render_model = (skr_live2d_render_model_async_t*)data;
                    render_model->buffer_finish(request);
                };
                ib_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = render_model;
                vram_service->request(&ib_io, &io_request, &buffer_destination);
                index_buffer_cursor += icount * sizeof(Csm::csmUint16);
            }
            else
            {
                auto& io_request = render_model->buffer_io_requests[i];
                io_request.status = SKR_ASYNC_IO_STATUS_OK;
                render_model->buffer_finish(&io_request);    
            }
        }
    }
}
#endif

CGPUTextureId skr_live2d_render_model_get_texture(skr_live2d_render_model_id render_model, uint32_t drawable_index)
{
    auto tIdx = render_model->model_resource_id->model->GetModel()->GetDrawableTextureIndex(drawable_index);
    return render_model->textures[tIdx];
}

CGPUTextureViewId skr_live2d_render_model_get_texture_view(skr_live2d_render_model_id render_model, uint32_t drawable_index)
{
    auto tIdx = render_model->model_resource_id->model->GetModel()->GetDrawableTextureIndex(drawable_index);
    return render_model->texture_views[tIdx];
}

void skr_live2d_render_model_free(skr_live2d_render_model_id render_model)
{
    SkrDelete(render_model);
}