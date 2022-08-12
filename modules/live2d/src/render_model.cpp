#include "skr_live2d/render_model.h"
#include "live2d_helpers.hpp"

struct skr_live2d_render_model_t {
    skr_live2d_model_resource_id model_resource_id;
    eastl::vector<skr_async_io_request_t> texture_requests;
    eastl::vector<skr_vram_buffer_request_t> buffer_requests;
    eastl::vector_map<uint32_t, skr_vertex_buffer_view_t> vertex_buffer_views;
    eastl::vector_map<uint32_t, skr_index_buffer_view_t> index_buffer_views;
};

bool skr_live2d_render_model_request_t::is_buffer_ready() const SKR_NOEXCEPT
{
    return get_buffer_status() == SKR_ASYNC_IO_STATUS_OK;
}

SkrAsyncIOStatus skr_live2d_render_model_request_t::get_buffer_status() const SKR_NOEXCEPT
{
    return (SkrAsyncIOStatus)skr_atomic32_load_acquire(&buffers_io_status);
}

void skr_live2d_render_model_create_from_raw(skr_io_ram_service_t* ram_service, skr_io_vram_service_t* vram_service,
    skr_live2d_model_resource_id resource, skr_live2d_render_model_t* request)
{
    // request load textures
    
}

void skr_live2d_render_model_free(skr_live2d_render_model_id render_model)
{

}