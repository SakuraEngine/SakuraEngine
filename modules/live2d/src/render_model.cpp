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

struct skr_live2d_render_model_t {
    ~skr_live2d_render_model_t() SKR_NOEXCEPT
    {
        for (auto&& request : texture_requests)
        {
            cgpu_free_texture(request.out_texture);
        }
    }

    skr_live2d_model_resource_id model_resource_id;
    eastl::vector<skr_async_io_request_t> texture_io_requests;
    eastl::vector<skr_vram_texture_request_t> texture_requests;
    eastl::vector<skr_async_io_request_t> buffer_io_requests;
    eastl::vector<skr_vram_buffer_request_t> buffer_requests;
    eastl::vector_map<uint32_t, skr_vertex_buffer_view_t> vertex_buffer_views;
    eastl::vector_map<uint32_t, skr_index_buffer_view_t> index_buffer_views;
};

struct skr_live2d_render_model_async_t : public skr_live2d_render_model_t {
    skr_live2d_render_model_async_t() = delete;
    skr_live2d_render_model_async_t(skr_live2d_render_model_request_t* request)
        : skr_live2d_render_model_t(), request(request)
    {

    }
    void texture_finish(skr_async_io_request_t* p_io_request)
    {

        try_finish();
    }
    void finish()
    {
        skr_atomic32_store_relaxed(&request->io_status, SKR_ASYNC_IO_STATUS_OK);
        request = nullptr;
    }
    void try_finish()
    {
        for (auto&& io_request : texture_io_requests)
        {
            if (!io_request.is_ready()) return;
        }
        for (auto&& io_request : buffer_io_requests)
        {
            if (!io_request.is_ready()) return;
        }
        finish();
    }
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
#ifndef _WIN32
    SKR_UNIMPLEMENTED_FUNCTION();
#endif
    auto csmModel = resource->model->GetModel();
    SKR_ASSERT(csmModel && "csmModel is null");
    // request load textures
    auto dstorage_queue = request->dstorage_queue_override;
    const uint32_t texture_count = resource->model_setting->GetTextureCount();
    auto render_model = request->render_model = SkrNew<skr_live2d_render_model_async_t>(request);
    render_model->texture_requests.resize(texture_count);
    render_model->texture_io_requests.resize(texture_count);
    if (request->dstorage_queue_override)
    {
        for (uint32_t i = 0; i < texture_count; i++)
        {
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
            vram_texture_io.dstorage_queue = dstorage_queue;
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
    // request load buffers
    // csmModel->GetDrawableCount()

}
#endif

void skr_live2d_render_model_free(skr_live2d_render_model_id render_model)
{
    SkrDelete(render_model);
}