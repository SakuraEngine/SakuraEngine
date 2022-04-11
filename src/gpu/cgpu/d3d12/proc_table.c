#include "cgpu/backend/d3d12/cgpu_d3d12.h"
#include "cgpu/backend/d3d12/cgpu_d3d12_surfaces.h"

const CGpuProcTable tbl_d3d12 = {
    // Instance APIs
    .create_instance = &cgpu_create_instance_d3d12,
    .query_instance_features = &cgpu_query_instance_features_d3d12,
    .free_instance = &cgpu_free_instance_d3d12,

    // Adapter APIs
    .enum_adapters = &cgpu_enum_adapters_d3d12,
    .query_adapter_detail = &cgpu_query_adapter_detail_d3d12,
    .query_queue_count = &cgpu_query_queue_count_d3d12,

    // Device APIs
    .create_device = &cgpu_create_device_d3d12,
    .free_device = &cgpu_free_device_d3d12,

    // API Object APIs
    .create_fence = &cgpu_create_fence_d3d12,
    .wait_fences = &cgpu_wait_fences_d3d12,
    .query_fence_status = &cgpu_query_fence_status_d3d12,
    .free_fence = &cgpu_free_fence_d3d12,
    .create_semaphore = &cgpu_create_semaphore_d3d12,
    .free_semaphore = &cgpu_free_semaphore_d3d12,
    .create_root_signature = &cgpu_create_root_signature_d3d12,
    .free_root_signature = &cgpu_free_root_signature_d3d12,
    .create_descriptor_set = &cgpu_create_descriptor_set_d3d12,
    .update_descriptor_set = &cgpu_update_descriptor_set_d3d12,
    .free_descriptor_set = &cgpu_free_descriptor_set_d3d12,
    .create_compute_pipeline = &cgpu_create_compute_pipeline_d3d12,
    .free_compute_pipeline = &cgpu_free_compute_pipeline_d3d12,
    .create_render_pipeline = &cgpu_create_render_pipeline_d3d12,
    .free_render_pipeline = &cgpu_free_render_pipeline_d3d12,

    // Queue APIs
    .get_queue = &cgpu_get_queue_d3d12,
    .submit_queue = &cgpu_submit_queue_d3d12,
    .wait_queue_idle = &cgpu_wait_queue_idle_d3d12,
    .queue_present = &cgpu_queue_present_d3d12,
    .free_queue = &cgpu_free_queue_d3d12,

    // Command APIs
    .create_command_pool = &cgpu_create_command_pool_d3d12,
    .create_command_buffer = &cgpu_create_command_buffer_d3d12,
    .reset_command_pool = &cgpu_reset_command_pool_d3d12,
    .free_command_buffer = &cgpu_free_command_buffer_d3d12,
    .free_command_pool = &cgpu_free_command_pool_d3d12,

    // Shader APIs
    .create_shader_library = &cgpu_create_shader_library_d3d12,
    .free_shader_library = &cgpu_free_shader_library_d3d12,

    // Texture/TextureView APIs
    .create_texture = &cgpu_create_texture_d3d12,
    .free_texture = &cgpu_free_texture_d3d12,
    .create_texture_view = &cgpu_create_texture_view_d3d12,
    .free_texture_view = &cgpu_free_texture_view_d3d12,

    // Buffer APIs
    .create_buffer = &cgpu_create_buffer_d3d12,
    .map_buffer = &cgpu_map_buffer_d3d12,
    .unmap_buffer = &cgpu_unmap_buffer_d3d12,
    .free_buffer = &cgpu_free_buffer_d3d12,

    // Sampler APIs
    .create_sampler = &cgpu_create_sampler_d3d12,
    .free_sampler = &cgpu_free_sampler_d3d12,

    // Swapchain APIs
    .create_swapchain = &cgpu_create_swapchain_d3d12,
    .acquire_next_image = &cgpu_acquire_next_image_d3d12,
    .free_swapchain = &cgpu_free_swapchain_d3d12,

    // CMDs
    .cmd_begin = &cgpu_cmd_begin_d3d12,
    .cmd_transfer_buffer_to_buffer = &cgpu_cmd_transfer_buffer_to_buffer_d3d12,
    .cmd_transfer_texture_to_texture = &cgpu_cmd_transfer_texture_to_texture_d3d12,
    .cmd_transfer_buffer_to_texture = &cgpu_cmd_transfer_buffer_to_texture_d3d12,
    .cmd_resource_barrier = &cgpu_cmd_resource_barrier_d3d12,
    .cmd_end = &cgpu_cmd_end_d3d12,

    // Compute Pass
    .cmd_begin_compute_pass = &cgpu_cmd_begin_compute_pass_d3d12,
    .compute_encoder_bind_descriptor_set = &cgpu_compute_encoder_bind_descriptor_set_d3d12,
    .compute_encoder_push_constants = &cgpu_compute_encoder_push_constants_d3d12,
    .compute_encoder_bind_pipeline = &cgpu_compute_encoder_bind_pipeline_d3d12,
    .compute_encoder_dispatch = &cgpu_compute_encoder_dispatch_d3d12,
    .cmd_end_compute_pass = &cgpu_cmd_end_compute_pass_d3d12,

    // Render Pass
    .cmd_begin_render_pass = &cgpu_cmd_begin_render_pass_d3d12,
    .render_encoder_bind_descriptor_set = cgpu_render_encoder_bind_descriptor_set_d3d12,
    .render_encoder_set_viewport = &cgpu_render_encoder_set_viewport_d3d12,
    .render_encoder_set_scissor = &cgpu_render_encoder_set_scissor_d3d12,
    .render_encoder_bind_pipeline = &cgpu_render_encoder_bind_pipeline_d3d12,
    .render_encoder_bind_vertex_buffers = &cgpu_render_encoder_bind_vertex_buffers_d3d12,
    .render_encoder_bind_index_buffer = &cgpu_render_encoder_bind_index_buffer_d3d12,
    .render_encoder_push_constants = &cgpu_render_encoder_push_constants_d3d12,
    .render_encoder_draw = &cgpu_render_encoder_draw_d3d12,
    .render_encoder_draw_instanced = &cgpu_render_encoder_draw_instanced_d3d12,
    .render_encoder_draw_indexed = &cgpu_render_encoder_draw_indexed_d3d12,
    .render_encoder_draw_indexed_instanced = &cgpu_render_encoder_draw_indexed_instanced_d3d12,
    .cmd_end_render_pass = &cgpu_cmd_end_render_pass_d3d12
};

const CGpuProcTable* CGPU_D3D12ProcTable()
{
    return &tbl_d3d12;
}

const CGpuSurfacesProcTable s_tbl_d3d12 = {
    //
    .free_surface = cgpu_free_surface_d3d12,
#if defined(_WIN32) || defined(_WIN64)
    .from_hwnd = cgpu_surface_from_hwnd_d3d12
#endif
    //
};

const CGpuSurfacesProcTable* CGPU_D3D12SurfacesProcTable()
{
    return &s_tbl_d3d12;
}
