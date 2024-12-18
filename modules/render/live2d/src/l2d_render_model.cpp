#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrCore/log.h"
#include "SkrRT/io/ram_io.hpp"
#include "SkrRT/io/vram_io.hpp"

#include "SkrCore/memory/memory.h"
#include "SkrBase/misc/debug.h" 
#include "SkrCore/platform/vfs.h"

#include "SkrLive2D/l2d_render_model.h"
#include "live2d_clipping.hpp"
#include "live2d_helpers.hpp"

#include <string> // TODO: replace this (std::stoi)
#include <SkrOS/filesystem.hpp>

#include "SkrImageCoder/skr_image_coder.h"
#ifdef _WIN32
#include "SkrImageCoder/extensions/win_dstorage_decompressor.h"
#endif

#include "SkrProfile/profile.h"

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

    skr::Vector<skr_io_future_t> png_futures;
    skr::Vector<skr_io_future_t> texture_futures;
    skr::Vector<skr::io::VRAMIOTextureId> io_textures;

    skr::Vector<skr_io_future_t> buffer_futures;
    skr::Vector<skr::io::VRAMIOBufferId> io_buffers;
};

struct skr_live2d_render_model_async_t : public skr_live2d_render_model_impl_t {
    skr_live2d_render_model_async_t() = delete;
    skr_live2d_render_model_async_t(skr_live2d_render_model_future_t* request, skr_live2d_model_resource_id model_resource)
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

    void texture_finish(skr_io_future_t* p_io_request)
    {
        finished_texture_request++;
        try_finish();
    }

    void buffer_finish(skr_io_future_t* p_io_request)
    {
        finished_buffer_request++;
        try_finish();
    }

    void finish()
    {
        for (uint32_t i = 0; i < textures.size(); i++)
        {
            textures[i] = io_textures[i]->get_texture();
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
        for (auto&& png_blob : png_blobs)
            png_blob.reset();
        for (auto&& decoder : decoders)
            decoder.reset();
        skr_atomic_store_relaxed(&request->io_status, SKR_IO_STAGE_COMPLETED);
        request = nullptr;
    }

    void try_finish()
    {
        if (finished_texture_request < texture_futures.size()) return;
        const auto bios = buffer_futures.size();
        if (finished_buffer_request < bios) return;
        finish();
    }

    uint32_t finished_texture_request = 0;
    uint32_t finished_buffer_request = 0;
    skr_live2d_render_model_future_t* request = nullptr;
    skr::io::IVRAMService* vram_service = nullptr;
    skr::io::IRAMService* ram_service = nullptr;
    CGPUDeviceId device;
    CGPUQueueId transfer_queue;
    skr::Vector<skr::ImageDecoderId> decoders;
    skr::Vector<skr::BlobId> png_blobs;
};

bool skr_live2d_render_model_future_t::is_ready() const SKR_NOEXCEPT
{
    return get_status() == SKR_IO_STAGE_COMPLETED;
}

ESkrIOStage skr_live2d_render_model_future_t::get_status() const SKR_NOEXCEPT
{
    return (ESkrIOStage)skr_atomic_load_acquire(&io_status);
}

#ifndef SKR_SERIALIZE_GURAD
void skr_live2d_render_model_create_from_raw(skr_io_ram_service_t* ram_service, skr_io_vram_service_t* vram_service, 
    CGPUDeviceId device, skr_live2d_model_resource_id resource, skr_live2d_render_model_future_t* request)
{
    auto csmModel = resource->model->GetModel();
    SKR_ASSERT(csmModel && "csmModel is null");
    const uint32_t texture_count = resource->model_setting->GetTextureCount();
    auto render_model = SkrNew<skr_live2d_render_model_async_t>(request, resource);
    request->render_model = render_model;
    render_model->use_dynamic_buffer = request->use_dynamic_buffer;
    render_model->device = device;
    render_model->transfer_queue = request->queue_override;
    render_model->vram_service = vram_service;
    render_model->ram_service = ram_service;
    // request load textures
    render_model->decoders.resize_default(texture_count);
    render_model->textures.resize_default(texture_count);
    render_model->texture_views.resize_default(texture_count);
    render_model->io_textures.resize_default(texture_count);
    render_model->texture_futures.resize_default(texture_count);
    [[maybe_unused]] const bool dstorage_available = vram_service->get_dstoage_available();
    if (!dstorage_available)
    {
        render_model->png_blobs.resize_default(texture_count);
        render_model->png_futures.resize_default(texture_count);
    }
    // request load buffers
    const auto use_dynamic_buffer = render_model->use_dynamic_buffer;
    const auto drawable_count = csmModel->GetDrawableCount();
    uint32_t total_index_count = 0;
    uint32_t total_vertex_count = 0;
    for (uint32_t i = 0; i < (uint32_t)drawable_count; i++)
    {
        const int32_t vcount = csmModel->GetDrawableVertexCount(i);
        total_vertex_count += vcount;
        const int32_t icount = csmModel->GetDrawableVertexIndexCount(i);
        total_index_count += icount;
    }
    // create index buffer
    {
        SkrZoneScopedN("CreateLive2DIndexBuffer");

        auto ib_desc = make_zeroed<CGPUBufferDescriptor>();
        skr::String name = (const char8_t*)resource->model_setting->GetModelFileName();
        auto ind_name = name;
        ind_name.append(u8"-i");
        ib_desc.name = ind_name.c_str();
        ib_desc.descriptors = CGPU_RESOURCE_TYPE_INDEX_BUFFER;
        ib_desc.flags = CGPU_BCF_NONE;
        ib_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
        ib_desc.size = total_index_count * sizeof(Csm::csmUint16);
        render_model->index_buffer = cgpu_create_buffer(device, &ib_desc);
    }
    // request textures 
    for (uint32_t i = 0; i < texture_count; i++)
    {
        SkrZoneScopedN("RequestLive2DTexture");

        SKR_UNUSED auto& texture = render_model->io_textures[i];
        SKR_UNUSED auto& texture_future = render_model->texture_futures[i];
        // SKR_UNUSED auto vram_texture_io = make_zeroed<skr_vram_texture_io_t>();
        const auto texture_path = resource->model_setting->GetTextureFileName(i);
        const auto pngPath = skr::filesystem::path(request->vfs_override->mount_dir) / resource->model->homePath.c_str() / texture_path;
        const auto pngPathStr = pngPath.u8string();
        const auto pngPathCStr = pngPathStr.c_str();
#ifdef _WIN32
        if (dstorage_available)
        {
            std::string p = texture_path;
            auto p1 = p.find(".");
            auto p2 = p.find("/");
            std::string number = p.substr(p1 + 1, p2 - p1 - 1);
            auto resolution = std::stoi(number);

            auto& texture_future = render_model->texture_futures[i];
            auto& io_texture = render_model->io_textures[i];
            auto request = vram_service->open_texture_request();
            CGPUTextureDescriptor tdesc = {};
            tdesc.name = (const char8_t*)texture_path;
            tdesc.descriptors = CGPU_RESOURCE_TYPE_TEXTURE;
            tdesc.width = resolution;
            tdesc.height = resolution;
            tdesc.depth = 1;
            tdesc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
            request->set_path(pngPathCStr);
            request->set_texture(render_model->device, &tdesc);
            request->set_transfer_queue(render_model->transfer_queue);
            request->set_dstorage_compression(SKR_WIN_DSTORAGE_COMPRESSION_TYPE_IMAGE, resolution * resolution * 4);
            request->add_callback(SKR_IO_STAGE_COMPLETED, 
            +[](skr_io_future_t* future, skr_io_request_t* request, void* data) {
                auto render_model = (skr_live2d_render_model_async_t*)data;
                render_model->texture_finish(future);
            }, render_model);
            io_texture = vram_service->request(request, &texture_future);
        }
        else
#endif
        {
            auto& png_future = render_model->png_futures[i];
            const auto on_complete = +[](skr_io_future_t* future, skr_io_request_t* request, void* data) noexcept {
                SkrZoneScopedN("Decode PNG");
                auto render_model = (skr_live2d_render_model_async_t*)data;
                auto idx = future - render_model->png_futures.data();
                auto png_blob = render_model->png_blobs[idx];
                const auto texture_path = render_model->model_resource_id->model_setting->GetTextureFileName((int32_t)idx);
                auto vram_service = render_model->vram_service;
                // decompress
                EImageCoderFormat format = skr_image_coder_detect_format((const uint8_t*)png_blob->get_data(), png_blob->get_size());
                auto decoder = skr::IImageDecoder::Create(format);
                render_model->decoders[idx] = decoder;
                if (decoder->initialize((const uint8_t*)png_blob->get_data(), png_blob->get_size()))
                {
                    const auto encoded_format = decoder->get_color_format();
                    const auto raw_format = (encoded_format == IMAGE_CODER_COLOR_FORMAT_BGRA) ? IMAGE_CODER_COLOR_FORMAT_RGBA : encoded_format;
                    SKR_LOG_TRACE(u8"image coder: width = %d, height = %d, encoded_size = %d, raw_size = %d", 
                        decoder->get_width(), decoder->get_height(), 
                        png_blob->get_size(), decoder->get_size());
                    if (decoder->decode(raw_format, decoder->get_bit_depth()))
                    {
                        // upload texture
                        auto& texture_future = render_model->texture_futures[idx];
                        auto& io_texture = render_model->io_textures[idx];

                        auto request = vram_service->open_texture_request();
                        CGPUTextureDescriptor tdesc = {};
                        tdesc.name = (const char8_t*)texture_path;
                        tdesc.descriptors = CGPU_RESOURCE_TYPE_TEXTURE;
                        tdesc.width = decoder->get_width();
                        tdesc.height = decoder->get_height();
                        tdesc.depth = 1;
                        tdesc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
                        request->set_texture(render_model->device, &tdesc);
                        request->set_transfer_queue(render_model->transfer_queue);
                        request->set_memory_src(decoder->get_data(), decoder->get_width() * decoder->get_height() * 4);
                        request->add_callback(SKR_IO_STAGE_COMPLETED, 
                        +[](skr_io_future_t* future, skr_io_request_t* request, void* data) {
                            auto render_model = (skr_live2d_render_model_async_t*)data;
                            render_model->texture_finish(future);
                        }, render_model);
                        io_texture = vram_service->request(request, &texture_future);
                    }
                }
            };
            auto ramrq = ram_service->open_request();
            ramrq->set_vfs(request->vfs_override);
            ramrq->set_path(pngPathCStr);
            ramrq->add_block({}); // read all
            ramrq->use_async_complete();
            ramrq->add_callback(SKR_IO_STAGE_COMPLETED, on_complete, render_model);
            auto blob = ram_service->request(ramrq, &png_future);
            render_model->png_blobs[i] = blob;
        }
    }
    // request indices I/O
    {
        render_model->buffer_futures.resize_zeroed(drawable_count);
        render_model->io_buffers.resize_zeroed(drawable_count);
        uint32_t index_buffer_cursor = 0;
        auto batch = vram_service->open_batch(drawable_count);
        for(uint32_t i = 0; i < (uint32_t)drawable_count; i++)
        {
            const int32_t icount = csmModel->GetDrawableVertexIndexCount(i);
            if (icount != 0)
            {
                SkrZoneScopedN("RequestLive2DIndexBuffer");

                const auto indices = csmModel->GetDrawableVertexIndices(i);
                auto& buffer_future = render_model->buffer_futures[i];
                auto& io_buffer = render_model->io_buffers[i];
                auto request = vram_service->open_buffer_request();
                request->set_buffer(render_model->index_buffer, index_buffer_cursor);
                request->set_transfer_queue(render_model->transfer_queue);
                request->set_memory_src((uint8_t*)indices, sizeof(Csm::csmUint16) * icount);
                request->add_callback(SKR_IO_STAGE_COMPLETED, 
                +[](skr_io_future_t* future, skr_io_request_t* request, void* data) {
                    auto render_model = (skr_live2d_render_model_async_t*)data;
                    render_model->buffer_finish(future);
                }, render_model);
                auto result = batch->add_request(request, &buffer_future);
                io_buffer = skr::static_pointer_cast<skr::io::IVRAMIOBuffer>(result);
                index_buffer_cursor += icount * sizeof(Csm::csmUint16);
            }
            else
            {
                auto& buffer_future = render_model->buffer_futures[i];
                buffer_future.status = SKR_IO_STAGE_COMPLETED;
                render_model->buffer_finish(&buffer_future);    
            }
        }
        if (!batch->get_requests().is_empty())
        {
            vram_service->request(batch);
        }
    }
    // create vertex buffer
    {
        SkrZoneScopedN("CreateLive2DVertexBuffer");

        auto vb_desc = make_zeroed<CGPUBufferDescriptor>();
        skr::String name = (const char8_t*)resource->model_setting->GetModelFileName();
        auto pos_name = name;
        pos_name.append(u8"-pos");
        vb_desc.name = pos_name.c_str();
        vb_desc.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
        vb_desc.flags = use_dynamic_buffer ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE;
        vb_desc.memory_usage = use_dynamic_buffer ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY;
        vb_desc.prefer_on_device = true;
        vb_desc.size = total_vertex_count * sizeof(skr_live2d_vertex_pos_t);
        render_model->pos_buffer = cgpu_create_buffer(device, &vb_desc);

        auto uv_name = name;
        uv_name.append(u8"-uv");
        vb_desc.name = uv_name.c_str();
        vb_desc.size = total_vertex_count * sizeof(skr_live2d_vertex_uv_t);
        render_model->uv_buffer = cgpu_create_buffer(device, &vb_desc);
    }
    // record vertex buffer view
    {
        render_model->vertex_buffer_views.resize_zeroed(2 * drawable_count);
        uint32_t pos_buffer_cursor = 0;
        uint32_t uv_buffer_cursor = 0;
        for(uint32_t i = 0; i < (uint32_t)drawable_count; i++)
        {
            const int32_t vcount = csmModel->GetDrawableVertexCount(i);
            if (vcount != 0)
            {
                SkrZoneScopedN("FillLive2DVertexBufferViews");
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
    // record index buffer view
    {
        render_model->primitive_commands.resize_zeroed(drawable_count);
        render_model->index_buffer_views.resize_zeroed(drawable_count);
        uint32_t index_buffer_cursor = 0;
        for(uint32_t i = 0; i < (uint32_t)drawable_count; i++)
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
            render_model->primitive_commands[i].vbvs = {&render_model->vertex_buffer_views[2 * i], 2};
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
