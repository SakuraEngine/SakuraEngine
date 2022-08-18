#include "platform/vfs.h"
#include "cgpu/io.hpp"
#include "utils/make_zeroed.hpp"
#include "platform/memory.h"
#include "platform/debug.h"
#include "skr_live2d/render_model.h"
#include "live2d_helpers.hpp"
#include <ghc/filesystem.hpp>

#ifdef _WIN32
#include "skr_image_coder/extensions/win_dstorage_decompressor.h"
#endif

#include "tracy/Tracy.hpp"

struct skr_live2d_render_model_impl_t : public skr_live2d_render_model_t {
    ~skr_live2d_render_model_impl_t() SKR_NOEXCEPT
    {
        for (auto&& request : texture_requests)
        {
            cgpu_free_texture(request.out_texture);
        }
        cgpu_free_buffer(index_buffer);
        cgpu_free_buffer(pos_buffer);
        cgpu_free_buffer(uv_buffer);
    }

    CGPUBufferId index_buffer;
    CGPUBufferId pos_buffer;
    CGPUBufferId uv_buffer;

    eastl::vector<skr_async_io_request_t> texture_io_requests;
    eastl::vector<skr_vram_texture_request_t> texture_requests;
    eastl::vector<skr_async_io_request_t> buffer_io_requests;
    eastl::vector<skr_vram_buffer_request_t> buffer_requests;
};

struct skr_live2d_render_model_async_t : public skr_live2d_render_model_impl_t {
    skr_live2d_render_model_async_t() = delete;
    skr_live2d_render_model_async_t(skr_live2d_render_model_request_t* request)
        : skr_live2d_render_model_impl_t(), request(request)
    {

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
        skr_atomic32_store_relaxed(&request->io_status, SKR_ASYNC_IO_STATUS_OK);
        request = nullptr;
    }
    void try_finish()
    {
        if (finished_texture_request < texture_io_requests.size()) return;
        if (finished_buffer_request < buffer_io_requests.size()) return;
        finish();
    }
    uint32_t finished_texture_request = 0;
    uint32_t finished_buffer_request = 0;
    bool use_dynamic_buffer = true;
    skr_live2d_render_model_request_t* request = nullptr;
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
    auto render_model = SkrNew<skr_live2d_render_model_async_t>(request);
    request->render_model = render_model;
#ifndef _WIN32
    SKR_UNIMPLEMENTED_FUNCTION();
#else
    // request load textures
    render_model->texture_requests.resize(texture_count);
    render_model->texture_io_requests.resize(texture_count);
    if (request->file_dstorage_queue_override)
    {
        for (uint32_t i = 0; i < texture_count; i++)
        {
            ZoneScopedN("RequestLive2DTexture");

            auto& texture_request = render_model->texture_requests[i];
            auto& texture_io_request = render_model->texture_io_requests[i];
            auto vram_texture_io = make_zeroed<skr_vram_texture_io_t>();
            auto texture_path = resource->model_setting->GetTextureFileName(i);
            std::string p = texture_path;
            int p1 = p.find(".");
            int p2 = p.find("/");
            std::string number = p.substr(p1 + 1, p2 - p1 - 1);
            auto resolution = std::stoi(number);
            vram_texture_io.device = device;
            vram_texture_io.dstorage_compression = SKR_WIN_DSTORAGE_COMPRESSION_TYPE_IMAGE;
            vram_texture_io.dstorage_source_type = CGPU_DSTORAGE_SOURCE_FILE;
            vram_texture_io.dstorage_queue = file_dstorage_queue;
            vram_texture_io.resource_types = CGPU_RESOURCE_TYPE_NONE;
            vram_texture_io.texture_name = texture_path;
            vram_texture_io.width = resolution;
            vram_texture_io.height = resolution;
            vram_texture_io.depth = 1;
            vram_texture_io.format = CGPU_FORMAT_R8G8B8A8_UINT;
            vram_texture_io.size = resolution * resolution * 4;
            auto pngPath = ghc::filesystem::path(request->vfs_override->mount_dir) / resource->model->homePath.c_str() / texture_path;
            auto pngPathStr = pngPath.u8string();
            vram_texture_io.path = pngPathStr.c_str();
            vram_texture_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data){
                auto render_model = (skr_live2d_render_model_async_t*)data;
                render_model->texture_finish(request);
            };
            vram_texture_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = render_model;
            vram_service->request(&vram_texture_io, &texture_io_request, &texture_request);
        }
    }
#endif
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
    // Record Vertex Buffer View
    render_model->pos_buffer_views.resize(drawable_count);
    render_model->uv_buffer_views.resize(drawable_count);
    uint32_t pos_buffer_cursor = 0;
    uint32_t uv_buffer_cursor = 0;
    for(uint32_t i = 0; i < drawable_count; i++)
    {
        const int32_t vcount = csmModel->GetDrawableVertexCount(i);
        if (vcount != 0)
        {
            ZoneScopedN("FillLive2DVertexBufferViews");
            render_model->pos_buffer_views[i].offset = pos_buffer_cursor;
            render_model->pos_buffer_views[i].stride = sizeof(skr_live2d_vertex_pos_t);
            render_model->pos_buffer_views[i].buffer = render_model->pos_buffer;
            pos_buffer_cursor += vcount * sizeof(skr_live2d_vertex_pos_t);

            render_model->uv_buffer_views[i].offset = uv_buffer_cursor;
            render_model->uv_buffer_views[i].stride = sizeof(skr_live2d_vertex_uv_t);
            render_model->uv_buffer_views[i].buffer = render_model->uv_buffer;
            uv_buffer_cursor += vcount * sizeof(skr_live2d_vertex_uv_t);
        }
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
    render_model->buffer_io_requests.resize(drawable_count);
    render_model->buffer_requests.resize(drawable_count);
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
            auto& buffer_request = render_model->buffer_requests[i];
            ib_io.dst_buffer = render_model->index_buffer;
            ib_io.offset = index_buffer_cursor;
            ib_io.dstorage_source_type = CGPU_DSTORAGE_SOURCE_MEMORY;
            ib_io.device = device;
            ib_io.dstorage_queue = request->queue_override ? nullptr : memory_dstorage_queue;
            ib_io.transfer_queue = request->queue_override;
            ib_io.resource_types = CGPU_RESOURCE_TYPE_INDEX_BUFFER;
            ib_io.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
            ib_io.buffer_size = total_index_count * sizeof(Csm::csmUint16);
            ib_io.bytes = (uint8_t*)indices;
            ib_io.size = sizeof(Csm::csmUint16) * icount;
            ib_io.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data){
                auto render_model = (skr_live2d_render_model_async_t*)data;
                render_model->buffer_finish(request);
            };
            ib_io.callback_datas[SKR_ASYNC_IO_STATUS_OK] = render_model;
            vram_service->request(&ib_io, &io_request, &buffer_request);
            index_buffer_cursor += icount * sizeof(Csm::csmUint16);
        }
    }
}
#endif

void skr_live2d_render_model_free(skr_live2d_render_model_id render_model)
{
    SkrDelete(render_model);
}