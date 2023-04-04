#include "cgpu/backend/vulkan/cgpu_vulkan.h"
#include "cgpu/backend/vulkan/cgpu_vulkan2.h"
#include "vulkan_utils.h"

const CGPUProcTable tbl_vk = {
    // Instance APIs
    .create_instance = &cgpu_create_instance_vulkan,
    .query_instance_features = &cgpu_query_instance_features_vulkan,
    .free_instance = &cgpu_free_instance_vulkan,

    // Adapter APIs
    .enum_adapters = &cgpu_enum_adapters_vulkan,
    .query_adapter_detail = &cgpu_query_adapter_detail_vulkan,
    .query_queue_count = &cgpu_query_queue_count_vulkan,

    // Device APIs
    .create_device = &cgpu_create_device_vulkan,
    .query_video_memory_info = &cgpu_query_video_memory_info_vulkan,
    .query_shared_memory_info = &cgpu_query_shared_memory_info_vulkan,
    .free_device = &cgpu_free_device_vulkan,

    // API Object APIs
    .create_fence = &cgpu_create_fence_vulkan,
    .wait_fences = &cgpu_wait_fences_vulkan,
    .query_fence_status = &cgpu_query_fence_status_vulkan,
    .free_fence = &cgpu_free_fence_vulkan,
    .create_semaphore = &cgpu_create_semaphore_vulkan,
    .free_semaphore = &cgpu_free_semaphore_vulkan,
    .create_root_signature = &cgpu_create_root_signature_vulkan,
    .free_root_signature = &cgpu_free_root_signature_vulkan,
    .create_root_signature_pool = &cgpu_create_root_signature_pool_vulkan,
    .free_root_signature_pool = &cgpu_free_root_signature_pool_vulkan,
    .create_descriptor_set = &cgpu_create_descriptor_set_vulkan,
    .update_descriptor_set = &cgpu_update_descriptor_set_vulkan,
    .free_descriptor_set = &cgpu_free_descriptor_set_vulkan,
    .create_compute_pipeline = &cgpu_create_compute_pipeline_vulkan,
    .free_compute_pipeline = &cgpu_free_compute_pipeline_vulkan,
    .create_render_pipeline = &cgpu_create_render_pipeline_vulkan,
    .free_render_pipeline = &cgpu_free_render_pipeline_vulkan,
    .create_query_pool = &cgpu_create_query_pool_vulkan,
    .free_query_pool = &cgpu_free_query_pool_vulkan,

    // Queue APIs
    .get_queue = &cgpu_get_queue_vulkan,
    .submit_queue = &cgpu_submit_queue_vulkan,
    .wait_queue_idle = &cgpu_wait_queue_idle_vulkan,
    .queue_present = &cgpu_queue_present_vulkan,
    .queue_get_timestamp_period = &cgpu_queue_get_timestamp_period_ns_vulkan,
    .free_queue = &cgpu_free_queue_vulkan,

    // Command APIs
    .create_command_pool = &cgpu_create_command_pool_vulkan,
    .create_command_buffer = &cgpu_create_command_buffer_vulkan,
    .reset_command_pool = &cgpu_reset_command_pool_vulkan,
    .free_command_buffer = &cgpu_free_command_buffer_vulkan,
    .free_command_pool = &cgpu_free_command_pool_vulkan,

    // Shader APIs
    .create_shader_library = &cgpu_create_shader_library_vulkan,
    .free_shader_library = &cgpu_free_shader_library_vulkan,

    // Buffer APIs
    .create_buffer = &cgpu_create_buffer_vulkan,
    .map_buffer = &cgpu_map_buffer_vulkan,
    .unmap_buffer = &cgpu_unmap_buffer_vulkan,
    .free_buffer = &cgpu_free_buffer_vulkan,

    // Texture/TextureView APIs
    .create_texture = &cgpu_create_texture_vulkan,
    .free_texture = &cgpu_free_texture_vulkan,
    .create_texture_view = &cgpu_create_texture_view_vulkan,
    .free_texture_view = &cgpu_free_texture_view_vulkan,
    .try_bind_aliasing_texture = &cgpu_try_bind_aliasing_texture_vulkan,

    // Shared Resource APIs
    .export_shared_texture_handle = &cgpu_export_shared_texture_handle_vulkan,
    .import_shared_texture_handle = &cgpu_import_shared_texture_handle_vulkan,

    // Sampler APIs
    .create_sampler = &cgpu_create_sampler_vulkan,
    .free_sampler = &cgpu_free_sampler_vulkan,

    // Swapchain APIs
    .create_swapchain = &cgpu_create_swapchain_vulkan,
    .acquire_next_image = &cgpu_acquire_next_image_vulkan,
    .free_swapchain = &cgpu_free_swapchain_vulkan,

    // CMDs
    .cmd_begin = &cgpu_cmd_begin_vulkan,
    .cmd_transfer_buffer_to_buffer = &cgpu_cmd_transfer_buffer_to_buffer_vulkan,
    .cmd_transfer_buffer_to_texture = &cgpu_cmd_transfer_buffer_to_texture_vulkan,
    .cmd_transfer_texture_to_texture = &cgpu_cmd_transfer_texture_to_texture_vulkan,
    .cmd_resource_barrier = &cgpu_cmd_resource_barrier_vulkan,
    .cmd_begin_query = &cgpu_cmd_begin_query_vulkan,
    .cmd_end_query = &cgpu_cmd_end_query_vulkan,
    .cmd_reset_query_pool = &cgpu_cmd_reset_query_pool_vulkan,
    .cmd_resolve_query = &cgpu_cmd_resolve_query_vulkan,
    .cmd_end = &cgpu_cmd_end_vulkan,

    // Events
    .cmd_begin_event = &cgpu_cmd_begin_event_vulkan,
    .cmd_set_marker = &cgpu_cmd_set_marker_vulkan,
    .cmd_end_event = &cgpu_cmd_end_event_vulkan,

    // Compute CMDs
    .cmd_begin_compute_pass = &cgpu_cmd_begin_compute_pass_vulkan,
    .compute_encoder_bind_descriptor_set = &cgpu_compute_encoder_bind_descriptor_set_vulkan,
    .compute_encoder_push_constants = &cgpu_compute_encoder_push_constants_vulkan,
    .compute_encoder_bind_pipeline = &cgpu_compute_encoder_bind_pipeline_vulkan,
    .compute_encoder_dispatch = &cgpu_compute_encoder_dispatch_vulkan,
    .cmd_end_compute_pass = &cgpu_cmd_end_compute_pass_vulkan,

    // Render CMDs
    .cmd_begin_render_pass = &cgpu_cmd_begin_render_pass_vulkan,
    .render_encoder_set_shading_rate = &cgpu_render_encoder_set_shading_rate_vulkan,
    .render_encoder_bind_descriptor_set = cgpu_render_encoder_bind_descriptor_set_vulkan,
    .render_encoder_set_viewport = &cgpu_render_encoder_set_viewport_vulkan,
    .render_encoder_set_scissor = &cgpu_render_encoder_set_scissor_vulkan,
    .render_encoder_bind_pipeline = &cgpu_render_encoder_bind_pipeline_vulkan,
    .render_encoder_bind_vertex_buffers = &cgpu_render_encoder_bind_vertex_buffers_vulkan,
    .render_encoder_bind_index_buffer = &cgpu_render_encoder_bind_index_buffer_vulkan,
    .render_encoder_push_constants = &cgpu_render_encoder_push_constants_vulkan,
    .render_encoder_draw = &cgpu_render_encoder_draw_vulkan,
    .render_encoder_draw_instanced = &cgpu_render_encoder_draw_instanced_vulkan,
    .render_encoder_draw_indexed = &cgpu_render_encoder_draw_indexed_vulkan,
    .render_encoder_draw_indexed_instanced = &cgpu_render_encoder_draw_indexed_instanced_vulkan,
    .cmd_end_render_pass = &cgpu_cmd_end_render_pass_vulkan,

    // Compiled/Linked ISA APIs
    .compile_and_link_shaders = &cgpu_compile_and_link_shaders_vulkan,
    .compile_shaders = &cgpu_compile_shaders_vulkan,
    .free_compiled_shader = &cgpu_free_compiled_shader_vulkan,
    .free_linked_shader = &cgpu_free_linked_shader_vulkan,

    // StateStream APIs
    .create_state_stream = &cgpu_create_state_stream_vulkan,
    .render_encoder_bind_state_stream = &cgpu_render_encoder_bind_state_stream_vulkan,
    .compute_encoder_bind_state_stream = &cgpu_compute_encoder_bind_state_stream_vulkan,
    .free_state_stream = &cgpu_free_state_stream_vulkan,

    // raster state encoder APIs
    .open_raster_state_encoder = &cgpu_open_raster_state_encoder_vulkan,
    .raster_state_encoder_set_viewport = &cgpu_raster_state_encoder_set_viewport_vulkan,
    .raster_state_encoder_set_scissor = &cgpu_raster_state_encoder_set_scissor_vulkan,
    .raster_state_encoder_set_cull_mode = &cgpu_raster_state_encoder_set_cull_mode_vulkan,
    .raster_state_encoder_set_front_face = &cgpu_raster_state_encoder_set_front_face_vulkan,
    .raster_state_encoder_set_primitive_topology = &cgpu_raster_state_encoder_set_primitive_topology_vulkan,
    .raster_state_encoder_set_depth_test_enabled = &cgpu_raster_state_encoder_set_depth_test_enabled_vulkan,
    .raster_state_encoder_set_depth_write_enabled = &cgpu_raster_state_encoder_set_depth_write_enabled_vulkan,
    .raster_state_encoder_set_stencil_compare_op = &cgpu_raster_state_encoder_set_stencil_compare_op_vulkan,
    .raster_state_encoder_set_fill_mode = &cgpu_raster_state_encoder_set_fill_mode_vulkan,
    .raster_state_encoder_set_sample_count = &cgpu_raster_state_encoder_set_sample_count_vulkan,
    .close_raster_state_encoder = &cgpu_close_raster_state_encoder_vulkan,

    // shader state encoder APIs
    .open_shader_state_encoder_r = &cgpu_open_shader_state_encoder_r_vulkan,
    .open_shader_state_encoder_c = &cgpu_open_shader_state_encoder_c_vulkan,
    .shader_state_encoder_bind_shaders = &cgpu_shader_state_encoder_bind_shaders_vulkan,
    .shader_state_encoder_bind_linked_shader = &cgpu_shader_state_encoder_bind_linked_shader_vulkan,
    .close_shader_state_encoder = &cgpu_close_shader_state_encoder_vulkan,

    // user ctx encoder APIs
    .open_user_state_encoder = &cgpu_open_user_state_encoder_vulkan,
    .close_user_state_encoder = &cgpu_close_user_state_encoder_vulkan,

    // binder APIs
    .create_binder = &cgpu_create_binder_vulkan,
    .binder_bind_vertex_layout = &cgpu_binder_bind_vertex_layout_vulkan,
    .binder_bind_vertex_buffer = &cgpu_binder_bind_vertex_buffer_vulkan,
    .free_binder = &cgpu_free_binder_vulkan
};
const CGPUProcTable* CGPU_VulkanProcTable() { return &tbl_vk; }

// GCGPUVkAllocationCallbacks
#include "tracy/TracyC.h"

static const char* kVulkanMemoryPoolNameUnknown = "vk::unknown";
static const char* kVulkanInternalMemoryPoolNames[5] = {
    "vk::command(internal)",
    "vk::object(internal)",
    "vk::cache(internal)",
    "vk::device(internal)",
    "vk::instance(internal)"
};
static const char* kVulkanMemoryPoolNames[5] = {
    "vk::command",
    "vk::object",
    "vk::cache",
    "vk::device",
    "vk::instance"
};

static void VKAPI_PTR cgpu_vulkan_internal_alloc_notify(
    void*                                       pUserData,
    size_t                                      size,
    VkInternalAllocationType                    allocationType,
    VkSystemAllocationScope                     allocationScope)
{

}

static void VKAPI_PTR cgpu_vulkan_internal_free_notify(
    void*                                       pUserData,
    size_t                                      size,
    VkInternalAllocationType                    allocationType,
    VkSystemAllocationScope                     allocationScope)
{

}

#define CGPU_ALIGN(size, align) ((size + align - 1) & (~(align - 1)))

typedef struct AllocHeader
{
    int16_t padding;
    int16_t scope;
    uint32_t alignment;
} AllocHeader;

static void* VKAPI_PTR cgpu_vulkan_alloc(
    void*                                       pUserData,
    size_t                                      size,
    size_t                                      alignment,
    VkSystemAllocationScope                     allocationScope)
{
    if (size == 0) return CGPU_NULLPTR;

    const char* CZoneN = "vulkan::alloc";
    TracyCZoneCS(z, SKR_ALLOC_TRACY_MARKER_COLOR, 16, 1);
    TracyCZoneText(z, CZoneN, strlen(CZoneN));
    TracyCZoneName(z, CZoneN, strlen(CZoneN));

    uint8_t* ptr = CGPU_NULLPTR;
    alignment = alignment ? alignment : _Alignof(AllocHeader);
    size_t padding =  CGPU_ALIGN(sizeof(AllocHeader), alignment);
    size_t aligned_size =  CGPU_ALIGN(size, alignment);
    if (allocationScope <= 4)
    {
        ptr = traced_os_malloc_aligned(aligned_size + padding, alignment, kVulkanMemoryPoolNames[allocationScope]);
    }
    else
    {
        ptr = traced_os_malloc_aligned(aligned_size + padding, alignment, kVulkanMemoryPoolNameUnknown);
    }
    uint8_t* result = (uint8_t*)ptr + padding; 
    AllocHeader* pHeader = (AllocHeader*)(result - sizeof(AllocHeader));
    pHeader->padding = (int16_t)padding;
    pHeader->alignment = (uint32_t)alignment;
    pHeader->scope = allocationScope;

    TracyCZoneEnd(z);

    return result;
}

static void VKAPI_PTR cgpu_vulkan_free(
    void*                                       pUserData,
    void*                                       pMemory)
{
    if (CGPU_NULLPTR == pMemory) return;

    const char* CZoneN = "vulkan::free";
    TracyCZoneCS(z, SKR_DEALLOC_TRACY_MARKER_COLOR, 16, 1);
    TracyCZoneText(z, CZoneN, strlen(CZoneN));
    TracyCZoneName(z, CZoneN, strlen(CZoneN));
    
    AllocHeader* pHeader = (AllocHeader*)((uint8_t*)pMemory - sizeof(AllocHeader));
    if (pHeader->scope <= 4)
    {
        traced_os_free_aligned((uint8_t*)pMemory - pHeader->padding, pHeader->alignment, kVulkanMemoryPoolNames[pHeader->scope]);
    }
    else
    {
        traced_os_free_aligned((uint8_t*)pMemory - pHeader->padding, pHeader->alignment, kVulkanMemoryPoolNameUnknown);
    }

    TracyCZoneEnd(z);
}

static void* VKAPI_PTR cgpu_vulkan_realloc(
    void*                                       pUserData,
    void*                                       pOriginal,
    size_t                                      size,
    size_t                                      alignment,
    VkSystemAllocationScope                     allocationScope)
{
    const char* CZoneN = "vulkan::realloc";
    TracyCZoneCS(z, SKR_ALLOC_TRACY_MARKER_COLOR, 16, 1);
    TracyCZoneText(z, CZoneN, strlen(CZoneN));
    TracyCZoneName(z, CZoneN, strlen(CZoneN));

    AllocHeader* pHeader = (AllocHeader*)((uint8_t*)pOriginal - sizeof(AllocHeader));
    alignment = alignment ? alignment : _Alignof(AllocHeader);
    size_t padding =  CGPU_ALIGN(sizeof(AllocHeader), alignment);
    size_t aligned_size =  CGPU_ALIGN(size, alignment);

    const char* PoolName = pHeader->scope <= 4 ? kVulkanMemoryPoolNames[pHeader->scope] : kVulkanMemoryPoolNameUnknown;
    void* ptr = traced_os_realloc_aligned((uint8_t*)pOriginal - pHeader->padding, 
        padding + aligned_size, alignment, PoolName);

    uint8_t* result = (uint8_t*)ptr + padding;
    pHeader = (AllocHeader*)(result - sizeof(AllocHeader));
    pHeader->padding = (int16_t)padding;
    pHeader->alignment = (uint32_t)alignment;
    pHeader->scope = allocationScope;

    TracyCZoneEnd(z);

    return result;
}

const VkAllocationCallbacks GCGPUVkAllocationCallbacks = {
    .pfnAllocation = &cgpu_vulkan_alloc,
    .pfnReallocation = &cgpu_vulkan_realloc,
    .pfnFree = &cgpu_vulkan_free,
    .pfnInternalAllocation = &cgpu_vulkan_internal_alloc_notify,
    .pfnInternalFree = &cgpu_vulkan_internal_free_notify,
    .pUserData = CGPU_NULLPTR
};

