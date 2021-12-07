#include "cgpu/api.h"
#include "runtime_table.h"
#ifdef CGPU_USE_VULKAN
    #include "cgpu/backend/vulkan/cgpu_vulkan.h"
#endif
#ifdef CGPU_USE_D3D12
    #include "cgpu/backend/d3d12/cgpu_d3d12.h"
#endif
#ifdef CGPU_USE_METAL
    #include "cgpu/backend/metal/cgpu_metal.h"
#endif
#ifdef __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_MAC
        #define _MACOS
        #if defined(ARCH_ARM64)
            #define TARGET_APPLE_ARM64
        #endif
    #endif
    #if TARGET_OS_IPHONE
        #define TARGET_IOS
    #endif
    #if TARGET_IPHONE_SIMULATOR
        #define TARGET_IOS_SIMULATOR
    #endif
#elif defined _WIN32 || defined _WIN64
#endif
#include "assert.h"

CGpuInstanceId cgpu_create_instance(const CGpuInstanceDescriptor* desc)
{
    assert((desc->backend == ECGPUBackEnd_VULKAN || desc->backend == ECGPUBackEnd_D3D12 || desc->backend == ECGPUBackEnd_METAL) && "cgpu support only vulkan & d3d12 currently!");
    const CGpuProcTable* tbl = CGPU_NULLPTR;
    const CGpuSurfacesProcTable* s_tbl = CGPU_NULLPTR;

    if (desc->backend == ECGPUBackEnd_COUNT)
    {
    }
#ifdef CGPU_USE_VULKAN
    else if (desc->backend == ECGPUBackEnd_VULKAN)
    {
        tbl = CGPU_VulkanProcTable();
        s_tbl = CGPU_VulkanSurfacesProcTable();
    }
#endif
#ifdef CGPU_USE_METAL
    else if (desc->backend == ECGPUBackEnd_METAL)
    {
        tbl = CGPU_MetalProcTable();
        s_tbl = CGPU_MetalSurfacesProcTable();
    }
#endif
#ifdef CGPU_USE_D3D12
    else if (desc->backend == ECGPUBackEnd_D3D12)
    {
        tbl = CGPU_D3D12ProcTable();
        s_tbl = CGPU_D3D12SurfacesProcTable();
    }
#endif
    CGpuInstance* instance = (CGpuInstance*)tbl->create_instance(desc);
    *(bool*)&instance->enable_set_name = desc->enable_set_name;
    instance->proc_table = tbl;
    instance->surfaces_table = s_tbl;
    instance->runtime_table = cgpu_create_runtime_table();
    return instance;
}

RUNTIME_API void cgpu_query_instance_features(CGpuInstanceId instance, struct CGpuInstanceFeatures* features)
{
    assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    assert(instance->proc_table->query_instance_features && "query_instance_features Proc Missing!");

    instance->proc_table->query_instance_features(instance, features);
}

void cgpu_free_instance(CGpuInstanceId instance)
{
    assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    assert(instance->proc_table->free_instance && "free_instance Proc Missing!");

    cgpu_free_runtime_table(instance->runtime_table);
    instance->proc_table->free_instance(instance);
}

void cgpu_enum_adapters(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num)
{
    assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    assert(instance->proc_table->enum_adapters && "enum_adapters Proc Missing!");

    instance->proc_table->enum_adapters(instance, adapters, adapters_num);
    // ++ proc_table_cache
    if (adapters != CGPU_NULLPTR)
    {
        for (uint32_t i = 0; i < *adapters_num; i++)
        {
            *(const CGpuProcTable**)&adapters[i]->proc_table_cache = instance->proc_table;
            *(CGpuInstanceId*)&adapters[i]->instance = instance;
        }
    }
    // -- proc_table_cache
}

const char* unknownAdapterName = "UNKNOWN";
const struct CGpuAdapterDetail* cgpu_query_adapter_detail(const CGpuAdapterId adapter)
{
    assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(adapter->proc_table_cache->query_adapter_detail && "query_adapter_detail Proc Missing!");

    CGpuAdapterDetail* detail = (CGpuAdapterDetail*)adapter->proc_table_cache->query_adapter_detail(adapter);
    return detail;
}

uint32_t cgpu_query_queue_count(const CGpuAdapterId adapter, const ECGpuQueueType type)
{
    assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(adapter->proc_table_cache->query_queue_count && "query_queue_count Proc Missing!");

    return adapter->proc_table_cache->query_queue_count(adapter, type);
}

CGpuDeviceId cgpu_create_device(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc)
{
    assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(adapter->proc_table_cache->create_device && "create_device Proc Missing!");

    CGpuDeviceId device = adapter->proc_table_cache->create_device(adapter, desc);
    // ++ proc_table_cache
    if (device != CGPU_NULLPTR)
    {
        *(const CGpuProcTable**)&device->proc_table_cache = adapter->proc_table_cache;
    }
    // -- proc_table_cache
    return device;
}

CGpuFenceId cgpu_create_fence(CGpuDeviceId device)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->create_fence && "create_fence Proc Missing!");
    CGpuFence* fence = (CGpuFence*)device->proc_table_cache->create_fence(device);
    fence->device = device;
    return fence;
}

void cgpu_free_fence(CGpuFenceId fence)
{
    assert(fence != CGPU_NULLPTR && "fatal: call on NULL fence!");
    assert(fence->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcFreeFence fn_free_fence = fence->device->proc_table_cache->free_fence;
    assert(fn_free_fence && "free_fence Proc Missing!");
    fn_free_fence(fence);
}

CGpuRootSignatureId cgpu_create_root_signature(CGpuDeviceId device, const struct CGpuRootSignatureDescriptor* desc)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->create_root_signature && "create_root_signature Proc Missing!");
    CGpuRootSignature* signature = (CGpuRootSignature*)device->proc_table_cache->create_root_signature(device, desc);
    signature->device = device;
    return signature;
}

void cgpu_free_root_signature(CGpuRootSignatureId signature)
{
    assert(signature != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGpuDeviceId device = signature->device;
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->create_root_signature && "free_root_signature Proc Missing!");
    device->proc_table_cache->free_root_signature(signature);
}

CGpuComputePipelineId cgpu_create_compute_pipeline(CGpuDeviceId device, const struct CGpuComputePipelineDescriptor* desc)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->create_compute_pipeline && "create_compute_pipeline Proc Missing!");
    CGpuComputePipeline* pipeline = (CGpuComputePipeline*)device->proc_table_cache->create_compute_pipeline(device, desc);
    pipeline->device = device;
    return pipeline;
}

void cgpu_free_compute_pipeline(CGpuComputePipelineId pipeline)
{
    assert(pipeline != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGpuDeviceId device = pipeline->device;
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->free_compute_pipeline && "free_compute_pipeline Proc Missing!");
    device->proc_table_cache->free_compute_pipeline(pipeline);
}

CGpuRenderPipelineId cgpu_create_render_pipeline(CGpuDeviceId device, const struct CGpuRenderPipelineDescriptor* desc)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->create_render_pipeline && "create_render_pipeline Proc Missing!");
    CGpuRenderPipeline* pipeline = (CGpuRenderPipeline*)device->proc_table_cache->create_render_pipeline(device, desc);
    pipeline->device = device;
    return pipeline;
}

void cgpu_free_render_pipeline(CGpuRenderPipelineId pipeline)
{
    assert(pipeline != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGpuDeviceId device = pipeline->device;
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->free_render_pipeline && "free_render_pipeline Proc Missing!");
    device->proc_table_cache->free_render_pipeline(pipeline);
}

void cgpu_free_device(CGpuDeviceId device)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->free_device && "free_device Proc Missing!");

    device->proc_table_cache->free_device(device);
    return;
}

CGpuQueueId cgpu_get_queue(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->free_device && "free_device Proc Missing!");

    CGpuQueueId created = cgpu_runtime_table_try_get_queue(device, type, index);
    if (created != NULL)
    {
        printf("[Warn] You should not call cgpu_get_queue "
               "with a specific index & type for multiple times!\n"
               "       Please get for only once and reuse the handle!\n");
        return created;
    }
    CGpuQueue* queue = (CGpuQueue*)device->proc_table_cache->get_queue(device, type, index);
    queue->index = index;
    queue->type = type;
    queue->device = device;
    cgpu_runtime_table_add_queue(queue, type, index);
    return queue;
}

void cgpu_submit_queue(CGpuQueueId queue, const struct CGpuQueueSubmitDescriptor* desc)
{
    assert(desc != CGPU_NULLPTR && "fatal: call on NULL desc!");
    assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcSubmitQueue submit_queue = queue->device->proc_table_cache->submit_queue;
    assert(submit_queue && "submit_queue Proc Missing!");

    submit_queue(queue, desc);
}

void cgpu_wait_queue_idle(CGpuQueueId queue)
{
    assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcWaitQueueIdle wait_queue_idle = queue->device->proc_table_cache->wait_queue_idle;
    assert(wait_queue_idle && "wait_queue_idle Proc Missing!");

    wait_queue_idle(queue);
}

void cgpu_free_queue(CGpuQueueId queue)
{
    assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(queue->device->proc_table_cache->free_queue && "free_queue Proc Missing!");

    queue->device->proc_table_cache->free_queue(queue);
    return;
}

RUNTIME_API CGpuCommandPoolId cgpu_create_command_pool(CGpuQueueId queue,
    const CGpuCommandPoolDescriptor* desc)
{
    assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(queue->device->proc_table_cache->create_command_pool && "create_command_pool Proc Missing!");

    CGpuCommandPool* pool = (CGpuCommandPool*)queue->device->proc_table_cache->create_command_pool(queue, desc);
    pool->queue = queue;
    return pool;
}

RUNTIME_API CGpuCommandBufferId cgpu_create_command_buffer(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc)
{
    assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    const CGpuDeviceId device = pool->queue->device;
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCreateCommandBuffer fn_create_cmd = device->proc_table_cache->create_command_buffer;
    assert(fn_create_cmd && "create_command_buffer Proc Missing!");

    CGpuCommandBuffer* cmd = (CGpuCommandBuffer*)fn_create_cmd(pool, desc);
    cmd->pool = pool;
    cmd->device = device;
    return cmd;
}

RUNTIME_API void cgpu_free_command_buffer(CGpuCommandBufferId cmd)
{
    assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    CGpuCommandPoolId pool = cmd->pool;
    assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    const CGpuDeviceId device = pool->queue->device;
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcFreeCommandBuffer fn_free_cmd = device->proc_table_cache->free_command_buffer;
    assert(fn_free_cmd && "free_command_buffer Proc Missing!");

    fn_free_cmd(cmd);
}

RUNTIME_API void cgpu_free_command_pool(CGpuCommandPoolId pool)
{
    assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    assert(pool->queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(pool->queue->device->proc_table_cache->free_command_pool && "free_command_pool Proc Missing!");

    pool->queue->device->proc_table_cache->free_command_pool(pool);
    return;
}

// CMDs
void cgpu_cmd_begin(CGpuCommandBufferId cmd)
{
    assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBegin fn_cmd_begin = cmd->device->proc_table_cache->cmd_begin;
    assert(fn_cmd_begin && "cmd_begin Proc Missing!");
    fn_cmd_begin(cmd);
}

void cgpu_cmd_update_buffer(CGpuCommandBufferId cmd, const struct CGpuBufferUpdateDescriptor* desc)
{
    assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdUpdateBuffer fn_cmd_update_buffer = cmd->device->proc_table_cache->cmd_update_buffer;
    assert(fn_cmd_update_buffer && "cmd_update_buffer Proc Missing!");
    fn_cmd_update_buffer(cmd, desc);
}

void cgpu_cmd_end(CGpuCommandBufferId cmd)
{
    assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdEnd fn_cmd_end = cmd->device->proc_table_cache->cmd_end;
    assert(fn_cmd_end && "cmd_end Proc Missing!");
    fn_cmd_end(cmd);
}

CGpuComputePassEncoderId cgpu_cmd_begin_compute_pass(CGpuCommandBufferId cmd, const struct CGpuComputePassDescriptor* desc)
{
    assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBeginComputePass fn_begin_compute_pass = cmd->device->proc_table_cache->cmd_begin_compute_pass;
    assert(fn_begin_compute_pass && "cmd_begin_compute_pass Proc Missing!");
    CGpuComputePassEncoderId ecd = (CGpuComputePassEncoderId)fn_begin_compute_pass(cmd);
    return ecd;
}

// Shader APIs
CGpuShaderLibraryId cgpu_create_shader_library(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->create_shader_library && "create_shader_library Proc Missing!");

    CGPUProcCreateShaderLibrary fn_create_shader_library = device->proc_table_cache->create_shader_library;
    CGpuShaderLibrary* shader = (CGpuShaderLibrary*)fn_create_shader_library(device, desc);
    shader->device = device;
    // handle name string
    const size_t str_len = strlen(desc->name);
    const size_t str_size = str_len + 1;
    shader->name = (char8_t*)cgpu_calloc(1, str_size * sizeof(char8_t));
    memcpy((void*)shader->name, desc->name, str_size);
    return shader;
}

void cgpu_free_shader_library(CGpuShaderLibraryId library)
{
    assert(library != CGPU_NULLPTR && "fatal: call on NULL shader library!");
    const CGpuDeviceId device = library->device;
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    // handle name string
    cgpu_free((void*)library->name);

    CGPUProcFreeShaderLibrary fn_free_shader_library = device->proc_table_cache->free_shader_library;
    assert(fn_free_shader_library && "free_shader_library Proc Missing!");
    fn_free_shader_library(library);
}

CGpuBufferId cgpu_create_buffer(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->create_buffer && "create_buffer Proc Missing!");

    CGPUProcCreateBuffer fn_create_buffer = device->proc_table_cache->create_buffer;
    CGpuBuffer* buffer = (CGpuBuffer*)fn_create_buffer(device, desc);
    buffer->device = device;
    return buffer;
}

void cgpu_map_buffer(CGpuBufferId buffer, const struct CGpuBufferRange* range)
{
    assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    const CGpuDeviceId device = buffer->device;
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->map_buffer && "map_buffer Proc Missing!");

    CGPUProcMapBuffer fn_map_buffer = device->proc_table_cache->map_buffer;
    fn_map_buffer(buffer, range);
}

void cgpu_unmap_buffer(CGpuBufferId buffer)
{
    assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    const CGpuDeviceId device = buffer->device;
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->unmap_buffer && "unmap_buffer Proc Missing!");

    CGPUProcUnmapBuffer fn_unmap_buffer = device->proc_table_cache->unmap_buffer;
    fn_unmap_buffer(buffer);
}

void cgpu_free_buffer(CGpuBufferId buffer)
{
    assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    const CGpuDeviceId device = buffer->device;
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");

    CGPUProcFreeBuffer fn_free_buffer = device->proc_table_cache->free_buffer;
    assert(fn_free_buffer && "free_buffer Proc Missing!");
    fn_free_buffer(buffer);
}

// SwapChain APIs
CGpuSwapChainId cgpu_create_swapchain(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->proc_table_cache->create_swapchain && "create_swapchain Proc Missing!");

    if (desc->presentQueues == CGPU_NULLPTR)
    {
        assert(desc->presentQueuesCount <= 0 &&
               "fatal cgpu_create_swapchain: queue array & queue coutn dismatch!");
    }
    else
    {
        assert(desc->presentQueuesCount > 0 &&
               "fatal cgpu_create_swapchain: queue array & queue coutn dismatch!");
    }
    CGpuSwapChain* swapchain = (CGpuSwapChain*)device->proc_table_cache->create_swapchain(device, desc);
    assert(swapchain && "fatal cgpu_create_swapchain: NULL swapchain id returned from backend.");
    swapchain->device = device;
    return swapchain;
}

void cgpu_free_swapchain(CGpuSwapChainId swapchain)
{
    assert(swapchain != CGPU_NULLPTR && "fatal: call on NULL swapchain!");
    assert(swapchain->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(swapchain->device->proc_table_cache->create_swapchain && "create_swapchain Proc Missing!");

    swapchain->device->proc_table_cache->free_swapchain(swapchain);
    return;
}

// surfaces
#if defined(_WIN32) || defined(_WIN64)
CGpuSurfaceId cgpu_surface_from_hwnd(CGpuDeviceId device, HWND window)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    assert(device->adapter->instance->surfaces_table->from_hwnd != CGPU_NULLPTR && "free_instance Proc Missing!");

    return device->adapter->instance->surfaces_table->from_hwnd(device, window);
}
#elif defined(_MACOS)
CGpuSurfaceId cgpu_surface_from_ns_view(CGpuDeviceId device, CGpuNSView* window)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    assert(device->adapter->instance->surfaces_table->from_ns_view != CGPU_NULLPTR && "free_instance Proc Missing!");

    return device->adapter->instance->surfaces_table->from_ns_view(device, window);
}
#endif

void cgpu_free_surface(CGpuDeviceId device, CGpuSurfaceId surface)
{
    assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    assert(device->adapter->instance->surfaces_table->free_surface != CGPU_NULLPTR && "free_instance Proc Missing!");

    device->adapter->instance->surfaces_table->free_surface(device, surface);
    return;
}
//