#include "cgpu/api.h"
#include "cgpu/flags.h"
#include "common_utils.h"
#include <stdint.h>
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

RUNTIME_API CGPUInstanceId cgpu_create_instance(const CGPUInstanceDescriptor* desc)
{
    cgpu_assert((desc->backend == CGPU_BACKEND_VULKAN || desc->backend == CGPU_BACKEND_D3D12 || desc->backend == CGPU_BACKEND_METAL) && "CGPU support only vulkan & d3d12 & metal currently!");
    const CGPUProcTable* tbl = CGPU_NULLPTR;
    const CGPUSurfacesProcTable* s_tbl = CGPU_NULLPTR;

    if (desc->backend == CGPU_BACKEND_COUNT)
    {
    }
#ifdef CGPU_USE_VULKAN
    else if (desc->backend == CGPU_BACKEND_VULKAN)
    {
        tbl = CGPU_VulkanProcTable();
        s_tbl = CGPU_VulkanSurfacesProcTable();
    }
#endif
#ifdef CGPU_USE_METAL
    else if (desc->backend == CGPU_BACKEND_METAL)
    {
        tbl = CGPU_MetalProcTable();
        s_tbl = CGPU_MetalSurfacesProcTable();
    }
#endif
#ifdef CGPU_USE_D3D12
    else if (desc->backend == CGPU_BACKEND_D3D12)
    {
        tbl = CGPU_D3D12ProcTable();
        s_tbl = CGPU_D3D12SurfacesProcTable();
    }
#endif
    CGPUInstance* instance = (CGPUInstance*)tbl->create_instance(desc);
    *(bool*)&instance->enable_set_name = desc->enable_set_name;
    instance->backend = desc->backend;
    instance->proc_table = tbl;
    instance->surfaces_table = s_tbl;
    if(!instance->runtime_table) instance->runtime_table = cgpu_create_runtime_table();
    return instance;
}

RUNTIME_API ECGPUBackend cgpu_instance_get_backend(CGPUInstanceId instance)
{
    return instance->backend;
}

RUNTIME_API void cgpu_query_instance_features(CGPUInstanceId instance, struct CGPUInstanceFeatures* features)
{
    cgpu_assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    cgpu_assert(instance->proc_table->query_instance_features && "query_instance_features Proc Missing!");

    instance->proc_table->query_instance_features(instance, features);
}

RUNTIME_API void cgpu_free_instance(CGPUInstanceId instance)
{
    cgpu_assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    cgpu_assert(instance->proc_table->free_instance && "free_instance Proc Missing!");

    struct CGPURuntimeTable* runtime_table = instance->runtime_table;
    cgpu_early_free_runtime_table(runtime_table);
    instance->proc_table->free_instance(instance);
    cgpu_free_runtime_table(runtime_table);
}

void cgpu_enum_adapters(CGPUInstanceId instance, CGPUAdapterId* const adapters, uint32_t* adapters_num)
{
    cgpu_assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    cgpu_assert(instance->proc_table->enum_adapters && "enum_adapters Proc Missing!");

    instance->proc_table->enum_adapters(instance, adapters, adapters_num);
    // ++ proc_table_cache
    if (adapters != CGPU_NULLPTR)
    {
        for (uint32_t i = 0; i < *adapters_num; i++)
        {
            *(const CGPUProcTable**)&adapters[i]->proc_table_cache = instance->proc_table;
            *(CGPUInstanceId*)&adapters[i]->instance = instance;
        }
    }
    // -- proc_table_cache
}

const char* unknownAdapterName = "UNKNOWN";
const struct CGPUAdapterDetail* cgpu_query_adapter_detail(const CGPUAdapterId adapter)
{
    cgpu_assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(adapter->proc_table_cache->query_adapter_detail && "query_adapter_detail Proc Missing!");

    CGPUAdapterDetail* detail = (CGPUAdapterDetail*)adapter->proc_table_cache->query_adapter_detail(adapter);
    return detail;
}

uint32_t cgpu_query_queue_count(const CGPUAdapterId adapter, const ECGPUQueueType type)
{
    cgpu_assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(adapter->proc_table_cache->query_queue_count && "query_queue_count Proc Missing!");

    return adapter->proc_table_cache->query_queue_count(adapter, type);
}

CGPUDeviceId cgpu_create_device(CGPUAdapterId adapter, const CGPUDeviceDescriptor* desc)
{
    cgpu_assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(adapter->proc_table_cache->create_device && "create_device Proc Missing!");

    CGPUDeviceId device = adapter->proc_table_cache->create_device(adapter, desc);
    ((CGPUDevice*)device)->next_texture_id = 0;
    // ++ proc_table_cache
    if (device != CGPU_NULLPTR)
    {
        *(const CGPUProcTable**)&device->proc_table_cache = adapter->proc_table_cache;
    }
    // -- proc_table_cache
    return device;
}

void cgpu_query_video_memory_info(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(device->proc_table_cache->query_video_memory_info && "query_video_memory_info Proc Missing!");

    device->proc_table_cache->query_video_memory_info(device, total, used_bytes);
}

void cgpu_query_shared_memory_info(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(device->proc_table_cache->query_video_memory_info && "query_shared_memory_info Proc Missing!");

    device->proc_table_cache->query_shared_memory_info(device, total, used_bytes);
}

CGPUFenceId cgpu_create_fence(CGPUDeviceId device)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_fence && "create_fence Proc Missing!");
    CGPUFence* fence = (CGPUFence*)device->proc_table_cache->create_fence(device);
    fence->device = device;
    return fence;
}

void cgpu_wait_fences(const CGPUFenceId* fences, uint32_t fence_count)
{
    if (fences == CGPU_NULLPTR || fence_count <= 0)
    {
        return;
    }
    CGPUFenceId fence = fences[0];
    cgpu_assert(fence != CGPU_NULLPTR && "fatal: call on NULL fence!");
    cgpu_assert(fence->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcWaitFences fn_wait_fences = fence->device->proc_table_cache->wait_fences;
    cgpu_assert(fn_wait_fences && "wait_fences Proc Missing!");
    fn_wait_fences(fences, fence_count);
}

ECGPUFenceStatus cgpu_query_fence_status(CGPUFenceId fence)
{
    cgpu_assert(fence != CGPU_NULLPTR && "fatal: call on NULL fence!");
    cgpu_assert(fence->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcQueryFenceStatus fn_query_fence = fence->device->proc_table_cache->query_fence_status;
    cgpu_assert(fn_query_fence && "query_fence_status Proc Missing!");
    return fn_query_fence(fence);
}

void cgpu_free_fence(CGPUFenceId fence)
{
    cgpu_assert(fence != CGPU_NULLPTR && "fatal: call on NULL fence!");
    cgpu_assert(fence->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcFreeFence fn_free_fence = fence->device->proc_table_cache->free_fence;
    cgpu_assert(fn_free_fence && "free_fence Proc Missing!");
    fn_free_fence(fence);
}

CGPUSemaphoreId cgpu_create_semaphore(CGPUDeviceId device)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_semaphore && "create_semaphore Proc Missing!");
    CGPUSemaphore* semaphore = (CGPUSemaphore*)device->proc_table_cache->create_semaphore(device);
    semaphore->device = device;
    return semaphore;
}

void cgpu_free_semaphore(CGPUSemaphoreId semaphore)
{
    cgpu_assert(semaphore != CGPU_NULLPTR && "fatal: call on NULL semaphore!");
    cgpu_assert(semaphore->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcFreeSemaphore fn_free_semaphore = semaphore->device->proc_table_cache->free_semaphore;
    cgpu_assert(fn_free_semaphore && "free_semaphore Proc Missing!");
    fn_free_semaphore(semaphore);
}

CGPURootSignatureId cgpu_create_root_signature(CGPUDeviceId device, const struct CGPURootSignatureDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_root_signature && "create_root_signature Proc Missing!");
    CGPURootSignature* signature = (CGPURootSignature*)device->proc_table_cache->create_root_signature(device, desc);
    signature->device = device;
    return signature;
}

void cgpu_free_root_signature(CGPURootSignatureId signature)
{
    cgpu_assert(signature != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGPUDeviceId device = signature->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_root_signature && "free_root_signature Proc Missing!");
    device->proc_table_cache->free_root_signature(signature);
}

CGPURootSignaturePoolId cgpu_create_root_signature_pool(CGPUDeviceId device, const struct CGPURootSignaturePoolDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_root_signature_pool && "create_root_signature_pool Proc Missing!");
    CGPURootSignaturePool* pool = (CGPURootSignaturePool*)device->proc_table_cache->create_root_signature_pool(device, desc);
    pool->device = device;
    return pool;
}

void cgpu_free_root_signature_pool(CGPURootSignaturePoolId pool)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL signature pool!");
    const CGPUDeviceId device = pool->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_root_signature_pool && "free_root_signature_pool Proc Missing!");
    device->proc_table_cache->free_root_signature_pool(pool);
}

CGPUDescriptorSetId cgpu_create_descriptor_set(CGPUDeviceId device, const struct CGPUDescriptorSetDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_descriptor_set && "create_descriptor_set Proc Missing!");
    CGPUDescriptorSet* set = (CGPUDescriptorSet*)device->proc_table_cache->create_descriptor_set(device, desc);
    set->root_signature = desc->root_signature;
    set->index = desc->set_index;
    return set;
}

void cgpu_update_descriptor_set(CGPUDescriptorSetId set, const struct CGPUDescriptorData* datas, uint32_t count)
{
    cgpu_assert(set != CGPU_NULLPTR && "fatal: call on NULL descriptor set!");
    const CGPUDeviceId device = set->root_signature->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->update_descriptor_set && "update_descriptor_set Proc Missing!");
    device->proc_table_cache->update_descriptor_set(set, datas, count);
}

void cgpu_free_descriptor_set(CGPUDescriptorSetId set)
{
    cgpu_assert(set != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGPUDeviceId device = set->root_signature->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_descriptor_set && "free_descriptor_set Proc Missing!");
    device->proc_table_cache->free_descriptor_set(set);
}

CGPUComputePipelineId cgpu_create_compute_pipeline(CGPUDeviceId device, const struct CGPUComputePipelineDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_compute_pipeline && "create_compute_pipeline Proc Missing!");
    CGPUComputePipeline* pipeline = (CGPUComputePipeline*)device->proc_table_cache->create_compute_pipeline(device, desc);
    pipeline->device = device;
    pipeline->root_signature = desc->root_signature;
    return pipeline;
}

void cgpu_free_compute_pipeline(CGPUComputePipelineId pipeline)
{
    cgpu_assert(pipeline != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGPUDeviceId device = pipeline->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_compute_pipeline && "free_compute_pipeline Proc Missing!");
    device->proc_table_cache->free_compute_pipeline(pipeline);
}

static const CGPUBlendStateDescriptor defaultBlendStateDesc = {
    .src_factors[0] = CGPU_BLEND_CONST_ONE,
    .dst_factors[0] = CGPU_BLEND_CONST_ZERO,
    .blend_modes[0] = CGPU_BLEND_MODE_ADD,
    .src_alpha_factors[0] = CGPU_BLEND_CONST_ONE,
    .dst_alpha_factors[0] = CGPU_BLEND_CONST_ZERO,
    .masks[0] = CGPU_COLOR_MASK_ALL,
    .independent_blend = false
};
static const CGPURasterizerStateDescriptor defaultRasterStateDesc = {
    .cull_mode = CGPU_CULL_MODE_BACK,
    .fill_mode = CGPU_FILL_MODE_SOLID,
    .front_face = CGPU_FRONT_FACE_CCW,
    .slope_scaled_depth_bias = 0.f,
    .enable_depth_clamp = false,
    .enable_scissor = false,
    .enable_multi_sample = false,
    .depth_bias = 0
};
static const CGPUDepthStateDescriptor defaultDepthStateDesc = {
    .depth_test = false,
    .depth_write = false,
    .stencil_test = false
};
CGPURenderPipelineId cgpu_create_render_pipeline(CGPUDeviceId device, const struct CGPURenderPipelineDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_render_pipeline && "create_render_pipeline Proc Missing!");
    CGPURenderPipelineDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(CGPURenderPipelineDescriptor));
    CGPURenderPipeline* pipeline = CGPU_NULLPTR;
    if (desc->sample_count == 0)
        new_desc.sample_count = 1;
    if (desc->blend_state == CGPU_NULLPTR)
        new_desc.blend_state = &defaultBlendStateDesc;
    if (desc->depth_state == CGPU_NULLPTR)
        new_desc.depth_state = &defaultDepthStateDesc;
    if (desc->rasterizer_state == CGPU_NULLPTR)
        new_desc.rasterizer_state = &defaultRasterStateDesc;
    pipeline = (CGPURenderPipeline*)device->proc_table_cache->create_render_pipeline(device, &new_desc);
    pipeline->device = device;
    pipeline->root_signature = desc->root_signature;
    return pipeline;
}

void cgpu_free_render_pipeline(CGPURenderPipelineId pipeline)
{
    cgpu_assert(pipeline != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGPUDeviceId device = pipeline->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_render_pipeline && "free_render_pipeline Proc Missing!");
    device->proc_table_cache->free_render_pipeline(pipeline);
}

CGPUQueryPoolId cgpu_create_query_pool(CGPUDeviceId device, const struct CGPUQueryPoolDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    CGPUProcCreateQueryPool fn_create_query_pool = device->proc_table_cache->create_query_pool;
    cgpu_assert(fn_create_query_pool && "create_query_pool Proc Missing!");
    CGPUQueryPool* query_pool = (CGPUQueryPool*)fn_create_query_pool(device, desc);
    query_pool->device = device;
    return query_pool;
}

void cgpu_free_query_pool(CGPUQueryPoolId pool)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    CGPUProcFreeQueryPool fn_free_query_pool = pool->device->proc_table_cache->free_query_pool;
    cgpu_assert(fn_free_query_pool && "free_query_pool Proc Missing!");
    fn_free_query_pool(pool);
}

void cgpu_free_device(CGPUDeviceId device)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_device && "free_device Proc Missing!");

    device->proc_table_cache->free_device(device);
    return;
}

CGPUQueueId cgpu_get_queue(CGPUDeviceId device, ECGPUQueueType type, uint32_t index)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_device && "free_device Proc Missing!");

    CGPUQueueId created = cgpu_runtime_table_try_get_queue(device, type, index);
    if (created != NULL)
    {
        cgpu_warn("You should not call cgpu_get_queue "
                  "with a specific index & type for multiple times!\n"
                  "       Please get for only once and reuse the handle!\n");
        return created;
    }
    CGPUQueue* queue = (CGPUQueue*)device->proc_table_cache->get_queue(device, type, index);
    queue->index = index;
    queue->type = type;
    queue->device = device;
    cgpu_runtime_table_add_queue(queue, type, index);
    return queue;
}

void cgpu_submit_queue(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc)
{
    cgpu_assert(desc != CGPU_NULLPTR && "fatal: call on NULL desc!");
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcSubmitQueue submit_queue = queue->device->proc_table_cache->submit_queue;
    cgpu_assert(submit_queue && "submit_queue Proc Missing!");

    submit_queue(queue, desc);
}

void cgpu_queue_present(CGPUQueueId queue, const struct CGPUQueuePresentDescriptor* desc)
{
    cgpu_assert(desc != CGPU_NULLPTR && "fatal: call on NULL desc!");
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcQueuePresent fn_queue_present = queue->device->proc_table_cache->queue_present;
    cgpu_assert(fn_queue_present && "queue_present Proc Missing!");

    fn_queue_present(queue, desc);
}

void cgpu_wait_queue_idle(CGPUQueueId queue)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcWaitQueueIdle wait_queue_idle = queue->device->proc_table_cache->wait_queue_idle;
    cgpu_assert(wait_queue_idle && "wait_queue_idle Proc Missing!");

    wait_queue_idle(queue);
}

float cgpu_queue_get_timestamp_period_ns(CGPUQueueId queue)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcQueueGetTimestampPeriodNS fn_get_timestamp_period = queue->device->proc_table_cache->queue_get_timestamp_period;
    cgpu_assert(fn_get_timestamp_period && "queue_get_timestamp_period Proc Missing!");

    return fn_get_timestamp_period(queue);
}

void cgpu_free_queue(CGPUQueueId queue)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->free_queue && "free_queue Proc Missing!");

    queue->device->proc_table_cache->free_queue(queue);
    return;
}

RUNTIME_API CGPUCommandPoolId cgpu_create_command_pool(CGPUQueueId queue,
const CGPUCommandPoolDescriptor* desc)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->create_command_pool && "create_command_pool Proc Missing!");

    CGPUCommandPool* pool = (CGPUCommandPool*)queue->device->proc_table_cache->create_command_pool(queue, desc);
    pool->queue = queue;
    return pool;
}

RUNTIME_API CGPUCommandBufferId cgpu_create_command_buffer(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    const CGPUDeviceId device = pool->queue->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCreateCommandBuffer fn_create_cmd = device->proc_table_cache->create_command_buffer;
    cgpu_assert(fn_create_cmd && "create_command_buffer Proc Missing!");

    CGPUCommandBuffer* cmd = (CGPUCommandBuffer*)fn_create_cmd(pool, desc);
    cmd->pool = pool;
    cmd->device = device;
    return cmd;
}

RUNTIME_API void cgpu_reset_command_pool(CGPUCommandPoolId pool)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(pool->queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(pool->queue->device->proc_table_cache->reset_command_pool && "reset_command_pool Proc Missing!");

    pool->queue->device->proc_table_cache->reset_command_pool(pool);
    return;
}

RUNTIME_API void cgpu_free_command_buffer(CGPUCommandBufferId cmd)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    CGPUCommandPoolId pool = cmd->pool;
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    const CGPUDeviceId device = pool->queue->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcFreeCommandBuffer fn_free_cmd = device->proc_table_cache->free_command_buffer;
    cgpu_assert(fn_free_cmd && "free_command_buffer Proc Missing!");

    fn_free_cmd(cmd);
}

RUNTIME_API void cgpu_free_command_pool(CGPUCommandPoolId pool)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(pool->queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(pool->queue->device->proc_table_cache->free_command_pool && "free_command_pool Proc Missing!");

    pool->queue->device->proc_table_cache->free_command_pool(pool);
    return;
}

// CMDs
void cgpu_cmd_begin(CGPUCommandBufferId cmd)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBegin fn_cmd_begin = cmd->device->proc_table_cache->cmd_begin;
    cgpu_assert(fn_cmd_begin && "cmd_begin Proc Missing!");
    fn_cmd_begin(cmd);
}

void cgpu_cmd_transfer_buffer_to_buffer(CGPUCommandBufferId cmd, const struct CGPUBufferToBufferTransfer* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->current_dispatch == CGPU_PIPELINE_TYPE_NONE && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(desc != CGPU_NULLPTR && "fatal: call on NULL cpy_desc!");
    cgpu_assert(desc->src != CGPU_NULLPTR && "fatal: call on NULL cpy_src!");
    cgpu_assert(desc->dst != CGPU_NULLPTR && "fatal: call on NULL cpy_dst!");
    const CGPUProcCmdTransferBufferToBuffer fn_cmd_transfer_buffer_to_buffer = cmd->device->proc_table_cache->cmd_transfer_buffer_to_buffer;
    cgpu_assert(fn_cmd_transfer_buffer_to_buffer && "cmd_transfer_buffer_to_buffer Proc Missing!");
    fn_cmd_transfer_buffer_to_buffer(cmd, desc);
}

void cgpu_cmd_transfer_buffer_to_texture(CGPUCommandBufferId cmd, const struct CGPUBufferToTextureTransfer* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->current_dispatch == CGPU_PIPELINE_TYPE_NONE && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(desc != CGPU_NULLPTR && "fatal: call on NULL cpy_desc!");
    cgpu_assert(desc->src != CGPU_NULLPTR && "fatal: call on NULL cpy_src!");
    cgpu_assert(desc->dst != CGPU_NULLPTR && "fatal: call on NULL cpy_dst!");
    const CGPUProcCmdTransferBufferToTexture fn_cmd_transfer_buffer_to_texture = cmd->device->proc_table_cache->cmd_transfer_buffer_to_texture;
    cgpu_assert(fn_cmd_transfer_buffer_to_texture && "cmd_transfer_buffer_to_texture Proc Missing!");
    fn_cmd_transfer_buffer_to_texture(cmd, desc);
}

void cgpu_cmd_transfer_texture_to_texture(CGPUCommandBufferId cmd, const struct CGPUTextureToTextureTransfer* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->current_dispatch == CGPU_PIPELINE_TYPE_NONE && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(desc != CGPU_NULLPTR && "fatal: call on NULL cpy_desc!");
    cgpu_assert(desc->src != CGPU_NULLPTR && "fatal: call on NULL cpy_src!");
    cgpu_assert(desc->dst != CGPU_NULLPTR && "fatal: call on NULL cpy_dst!");
    const CGPUProcCmdTransferTextureToTexture fn_cmd_transfer_texture_to_texture = cmd->device->proc_table_cache->cmd_transfer_texture_to_texture;
    cgpu_assert(fn_cmd_transfer_texture_to_texture && "cmd_transfer_texture_to_texture Proc Missing!");
    fn_cmd_transfer_texture_to_texture(cmd, desc);
}

void cgpu_cmd_resource_barrier(CGPUCommandBufferId cmd, const struct CGPUResourceBarrierDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->current_dispatch == CGPU_PIPELINE_TYPE_NONE && "fatal: can't call resource barriers in render/dispatch passes!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdResourceBarrier fn_cmd_resource_barrier = cmd->device->proc_table_cache->cmd_resource_barrier;
    cgpu_assert(fn_cmd_resource_barrier && "cmd_resource_barrier Proc Missing!");
    fn_cmd_resource_barrier(cmd, desc);
}

void cgpu_cmd_begin_query(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBeginQuery fn_cmd_begin_query = cmd->device->proc_table_cache->cmd_begin_query;
    cgpu_assert(fn_cmd_begin_query && "cmd_begin_query Proc Missing!");
    fn_cmd_begin_query(cmd, pool, desc);
}

void cgpu_cmd_end_query(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdEndQuery fn_cmd_end_query = cmd->device->proc_table_cache->cmd_end_query;
    cgpu_assert(fn_cmd_end_query && "cmd_end_query Proc Missing!");
    fn_cmd_end_query(cmd, pool, desc);
}

void cgpu_cmd_reset_query_pool(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, uint32_t start_query, uint32_t query_count)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    CGPUProcCmdResetQueryPool fn_reset_query_pool = pool->device->proc_table_cache->cmd_reset_query_pool;
    cgpu_assert(fn_reset_query_pool && "reset_query_pool Proc Missing!");
    fn_reset_query_pool(cmd, pool, start_query, query_count);
}

void cgpu_cmd_resolve_query(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, CGPUBufferId readback, uint32_t start_query, uint32_t query_count)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdResolveQuery fn_cmd_resolve_query = cmd->device->proc_table_cache->cmd_resolve_query;
    cgpu_assert(fn_cmd_resolve_query && "cmd_resolve_query Proc Missing!");
    fn_cmd_resolve_query(cmd, pool, readback, start_query, query_count);
}

void cgpu_cmd_end(CGPUCommandBufferId cmd)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdEnd fn_cmd_end = cmd->device->proc_table_cache->cmd_end;
    cgpu_assert(fn_cmd_end && "cmd_end Proc Missing!");
    fn_cmd_end(cmd);
}

// Compute CMDs
CGPUComputePassEncoderId cgpu_cmd_begin_compute_pass(CGPUCommandBufferId cmd, const struct CGPUComputePassDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBeginComputePass fn_begin_compute_pass = cmd->device->proc_table_cache->cmd_begin_compute_pass;
    cgpu_assert(fn_begin_compute_pass && "cmd_begin_compute_pass Proc Missing!");
    CGPUComputePassEncoderId ecd = (CGPUComputePassEncoderId)fn_begin_compute_pass(cmd, desc);
    CGPUCommandBuffer* Cmd = (CGPUCommandBuffer*)cmd;
    Cmd->current_dispatch = CGPU_PIPELINE_TYPE_COMPUTE;
    return ecd;
}

void cgpu_compute_encoder_bind_descriptor_set(CGPUComputePassEncoderId encoder, CGPUDescriptorSetId set)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL compute encoder!");
    cgpu_assert(set != CGPU_NULLPTR && "fatal: call on NULL descriptor!");
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcComputeEncoderBindDescriptorSet fn_bind_descriptor_set = device->proc_table_cache->compute_encoder_bind_descriptor_set;
    cgpu_assert(fn_bind_descriptor_set && "compute_encoder_bind_descriptor_set Proc Missing!");
    fn_bind_descriptor_set(encoder, set);
}

void cgpu_compute_encoder_push_constants(CGPUComputePassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data)
{
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcComputeEncoderPushConstants fn_push_constants = device->proc_table_cache->compute_encoder_push_constants;
    cgpu_assert(fn_push_constants && "compute_encoder_push_constants Proc Missing!");
    fn_push_constants(encoder, rs, name, data);
}

void cgpu_compute_encoder_bind_pipeline(CGPUComputePassEncoderId encoder, CGPUComputePipelineId pipeline)
{
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcComputeEncoderBindPipeline fn_compute_bind_pipeline = device->proc_table_cache->compute_encoder_bind_pipeline;
    cgpu_assert(fn_compute_bind_pipeline && "compute_encoder_bind_pipeline Proc Missing!");
    fn_compute_bind_pipeline(encoder, pipeline);
}

void cgpu_compute_encoder_dispatch(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z)
{
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcComputeEncoderDispatch fn_compute_dispatch = device->proc_table_cache->compute_encoder_dispatch;
    cgpu_assert(fn_compute_dispatch && "compute_encoder_dispatch Proc Missing!");
    fn_compute_dispatch(encoder, X, Y, Z);
}

void cgpu_cmd_end_compute_pass(CGPUCommandBufferId cmd, CGPUComputePassEncoderId encoder)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(cmd->current_dispatch == CGPU_PIPELINE_TYPE_COMPUTE && "fatal: can't call end command pass on commnd buffer while not dispatching compute!");
    const CGPUProcCmdEndComputePass fn_end_compute_pass = cmd->device->proc_table_cache->cmd_end_compute_pass;
    cgpu_assert(fn_end_compute_pass && "cmd_end_compute_pass Proc Missing!");
    fn_end_compute_pass(cmd, encoder);
    CGPUCommandBuffer* Cmd = (CGPUCommandBuffer*)cmd;
    Cmd->current_dispatch = CGPU_PIPELINE_TYPE_NONE;
}

// Render CMDs
CGPURenderPassEncoderId cgpu_cmd_begin_render_pass(CGPUCommandBufferId cmd, const struct CGPURenderPassDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBeginRenderPass fn_begin_render_pass = cmd->device->proc_table_cache->cmd_begin_render_pass;
    cgpu_assert(fn_begin_render_pass && "cmd_begin_render_pass Proc Missing!");
    CGPURenderPassEncoderId ecd = (CGPURenderPassEncoderId)fn_begin_render_pass(cmd, desc);
    CGPUCommandBuffer* Cmd = (CGPUCommandBuffer*)cmd;
    Cmd->current_dispatch = CGPU_PIPELINE_TYPE_GRAPHICS;
    return ecd;
}

void cgpu_render_encoder_set_shading_rate(CGPURenderPassEncoderId encoder, ECGPUShadingRate shading_rate, ECGPUShadingRateCombiner post_rasterizer_rate, ECGPUShadingRateCombiner final_rate)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL compute encoder!");
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderSetShadingRate fn_set_shading_rate = device->proc_table_cache->render_encoder_set_shading_rate;
    cgpu_assert(fn_set_shading_rate && "render_encoder_set_shading_rate Proc Missing!");
    fn_set_shading_rate(encoder, shading_rate, post_rasterizer_rate, final_rate);
}

void cgpu_render_encoder_bind_descriptor_set(CGPURenderPassEncoderId encoder, CGPUDescriptorSetId set)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL compute encoder!");
    cgpu_assert(set != CGPU_NULLPTR && "fatal: call on NULL descriptor!");
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderBindDescriptorSet fn_bind_descriptor_set = device->proc_table_cache->render_encoder_bind_descriptor_set;
    cgpu_assert(fn_bind_descriptor_set && "render_encoder_bind_descriptor_set Proc Missing!");
    fn_bind_descriptor_set(encoder, set);
}

void cgpu_render_encoder_bind_vertex_buffers(CGPURenderPassEncoderId encoder, uint32_t buffer_count,
const CGPUBufferId* buffers, const uint32_t* strides, const uint32_t* offsets)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL compute encoder!");
    cgpu_assert(buffers != CGPU_NULLPTR && "fatal: call on NULL buffers!");
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRendeEncoderBindVertexBuffers fn_bind_vertex_buffers = device->proc_table_cache->render_encoder_bind_vertex_buffers;
    cgpu_assert(fn_bind_vertex_buffers && "render_encoder_bind_vertex_buffers Proc Missing!");
    fn_bind_vertex_buffers(encoder, buffer_count, buffers, strides, offsets);
}

RUNTIME_API void cgpu_render_encoder_bind_index_buffer(CGPURenderPassEncoderId encoder, CGPUBufferId buffer, uint32_t index_stride, uint64_t offset)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL compute encoder!");
    cgpu_assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRendeEncoderBindIndexBuffer fn_bind_index_buffer = device->proc_table_cache->render_encoder_bind_index_buffer;
    cgpu_assert(fn_bind_index_buffer && "render_encoder_bind_index_buffer Proc Missing!");
    fn_bind_index_buffer(encoder, buffer, index_stride, offset);
}

void cgpu_render_encoder_set_viewport(CGPURenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderSetViewport fn_render_set_viewport = device->proc_table_cache->render_encoder_set_viewport;
    cgpu_assert(fn_render_set_viewport && "render_encoder_set_viewport Proc Missing!");
    fn_render_set_viewport(encoder, x, y, width, height, min_depth, max_depth);
}

void cgpu_render_encoder_set_scissor(CGPURenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderSetScissor fn_render_set_scissor = device->proc_table_cache->render_encoder_set_scissor;
    cgpu_assert(fn_render_set_scissor && "render_encoder_set_scissor Proc Missing!");
    fn_render_set_scissor(encoder, x, y, width, height);
}

void cgpu_render_encoder_bind_pipeline(CGPURenderPassEncoderId encoder, CGPURenderPipelineId pipeline)
{
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderBindPipeline fn_render_bind_pipeline = device->proc_table_cache->render_encoder_bind_pipeline;
    cgpu_assert(fn_render_bind_pipeline && "render_encoder_bind_pipeline Proc Missing!");
    fn_render_bind_pipeline(encoder, pipeline);
}

void cgpu_render_encoder_push_constants(CGPURenderPassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data)
{
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderPushConstants fn_push_constants = device->proc_table_cache->render_encoder_push_constants;
    cgpu_assert(fn_push_constants && "render_encoder_push_constants Proc Missing!");
    fn_push_constants(encoder, rs, name, data);
}

void cgpu_render_encoder_draw(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex)
{
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderDraw fn_draw = device->proc_table_cache->render_encoder_draw;
    cgpu_assert(fn_draw && "render_encoder_draw Proc Missing!");
    fn_draw(encoder, vertex_count, first_vertex);
}

void cgpu_render_encoder_draw_instanced(CGPURenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex, uint32_t instance_count, uint32_t first_instance)
{
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderDrawInstanced fn_draw_instanced = device->proc_table_cache->render_encoder_draw_instanced;
    cgpu_assert(fn_draw_instanced && "render_encoder_draw_instanced Proc Missing!");
    fn_draw_instanced(encoder, vertex_count, first_vertex, instance_count, first_instance);
}

void cgpu_render_encoder_draw_indexed(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t first_vertex)
{
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderDrawIndexed fn_draw_indexed = device->proc_table_cache->render_encoder_draw_indexed;
    cgpu_assert(fn_draw_indexed && "render_encoder_draw_indexed Proc Missing!");
    fn_draw_indexed(encoder, index_count, first_index, first_vertex);
}

void cgpu_render_encoder_draw_indexed_instanced(CGPURenderPassEncoderId encoder, uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
{
    CGPUDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderDrawIndexedInstanced fn_draw_indexed_instanced = device->proc_table_cache->render_encoder_draw_indexed_instanced;
    cgpu_assert(fn_draw_indexed_instanced && "render_encoder_draw_indexed_instanced Proc Missing!");
    fn_draw_indexed_instanced(encoder, index_count, first_index, instance_count, first_instance, first_vertex);
}

void cgpu_cmd_end_render_pass(CGPUCommandBufferId cmd, CGPURenderPassEncoderId encoder)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(cmd->current_dispatch == CGPU_PIPELINE_TYPE_GRAPHICS && "fatal: can't call end command pass on commnd buffer while not dispatching graphics!");
    const CGPUProcCmdEndRenderPass fn_end_render_pass = cmd->device->proc_table_cache->cmd_end_render_pass;
    cgpu_assert(fn_end_render_pass && "cmd_end_render_pass Proc Missing!");
    fn_end_render_pass(cmd, encoder);
    CGPUCommandBuffer* Cmd = (CGPUCommandBuffer*)cmd;
    Cmd->current_dispatch = CGPU_PIPELINE_TYPE_NONE;
}

// Events
void cgpu_cmd_begin_event(CGPUCommandBufferId cmd, const CGPUEventInfo* event)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBeginEvent fn_begin_event = cmd->device->proc_table_cache->cmd_begin_event;
    fn_begin_event(cmd, event);
}

void cgpu_cmd_set_marker(CGPUCommandBufferId cmd, const CGPUMarkerInfo* marker)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdSetMarker fn_cmd_set_marker = cmd->device->proc_table_cache->cmd_set_marker;
    fn_cmd_set_marker(cmd, marker);
}

void cgpu_cmd_end_event(CGPUCommandBufferId cmd)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdEndEvent fn_end_event = cmd->device->proc_table_cache->cmd_end_event;
    fn_end_event(cmd);
}

// Shader APIs
CGPUShaderLibraryId cgpu_create_shader_library(CGPUDeviceId device, const struct CGPUShaderLibraryDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_shader_library && "create_shader_library Proc Missing!");

    CGPUProcCreateShaderLibrary fn_create_shader_library = device->proc_table_cache->create_shader_library;
    CGPUShaderLibrary* shader = (CGPUShaderLibrary*)fn_create_shader_library(device, desc);
    shader->device = device;
    // handle name string
    const size_t str_len = strlen(desc->name);
    const size_t str_size = str_len + 1;
    shader->name = (char8_t*)cgpu_calloc(1, str_size * sizeof(char8_t));
    memcpy((void*)shader->name, desc->name, str_size);
    return shader;
}

void cgpu_free_shader_library(CGPUShaderLibraryId library)
{
    cgpu_assert(library != CGPU_NULLPTR && "fatal: call on NULL shader library!");
    const CGPUDeviceId device = library->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    // handle name string
    cgpu_free((void*)library->name);

    CGPUProcFreeShaderLibrary fn_free_shader_library = device->proc_table_cache->free_shader_library;
    cgpu_assert(fn_free_shader_library && "free_shader_library Proc Missing!");
    fn_free_shader_library(library);
}

// Buffer APIs
CGPUBufferId cgpu_create_buffer(CGPUDeviceId device, const struct CGPUBufferDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_buffer && "create_buffer Proc Missing!");
    CGPUBufferDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(CGPUBufferDescriptor));
    if (desc->flags == 0)
    {
        new_desc.flags |= CGPU_BCF_NONE;
    }
    CGPUProcCreateBuffer fn_create_buffer = device->proc_table_cache->create_buffer;
    CGPUBuffer* buffer = (CGPUBuffer*)fn_create_buffer(device, &new_desc);
    buffer->device = device;
    return buffer;
}

void cgpu_map_buffer(CGPUBufferId buffer, const struct CGPUBufferRange* range)
{
    cgpu_assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    const CGPUDeviceId device = buffer->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->map_buffer && "map_buffer Proc Missing!");

    CGPUProcMapBuffer fn_map_buffer = device->proc_table_cache->map_buffer;
    fn_map_buffer(buffer, range);
}

void cgpu_unmap_buffer(CGPUBufferId buffer)
{
    cgpu_assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    const CGPUDeviceId device = buffer->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->unmap_buffer && "unmap_buffer Proc Missing!");

    CGPUProcUnmapBuffer fn_unmap_buffer = device->proc_table_cache->unmap_buffer;
    fn_unmap_buffer(buffer);
}

void cgpu_free_buffer(CGPUBufferId buffer)
{
    cgpu_assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    const CGPUDeviceId device = buffer->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");

    CGPUProcFreeBuffer fn_free_buffer = device->proc_table_cache->free_buffer;
    cgpu_assert(fn_free_buffer && "free_buffer Proc Missing!");
    fn_free_buffer(buffer);
}

// Texture/TextureView APIs
CGPUTextureId cgpu_create_texture(CGPUDeviceId device, const struct CGPUTextureDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_texture && "create_texture Proc Missing!");
    CGPUTextureDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(CGPUTextureDescriptor));
    if (desc->array_size == 0) new_desc.array_size = 1;
    if (desc->mip_levels == 0) new_desc.mip_levels = 1;
    if (desc->depth == 0) new_desc.depth = 1;
    if (desc->sample_count == 0) new_desc.sample_count = 1;
    CGPUProcCreateTexture fn_create_texture = device->proc_table_cache->create_texture;
    CGPUTexture* texture = (CGPUTexture*)fn_create_texture(device, &new_desc);
    texture->device = device;
    texture->sample_count = desc->sample_count;
    return texture;
}

void cgpu_free_texture(CGPUTextureId texture)
{
    cgpu_assert(texture != CGPU_NULLPTR && "fatal: call on NULL texture!");
    const CGPUDeviceId device = texture->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");

    CGPUProcFreeTexture fn_free_texture = device->proc_table_cache->free_texture;
    cgpu_assert(fn_free_texture && "free_texture Proc Missing!");
    fn_free_texture(texture);
}

CGPUSamplerId cgpu_create_sampler(CGPUDeviceId device, const struct CGPUSamplerDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_sampler && "create_sampler Proc Missing!");
    CGPUProcCreateSampler fn_create_sampler = device->proc_table_cache->create_sampler;
    CGPUSampler* sampler = (CGPUSampler*)fn_create_sampler(device, desc);
    sampler->device = device;
    return sampler;
}

void cgpu_free_sampler(CGPUSamplerId sampler)
{
    cgpu_assert(sampler != CGPU_NULLPTR && "fatal: call on NULL sampler!");
    const CGPUDeviceId device = sampler->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");

    CGPUProcFreeSampler fn_free_sampler = device->proc_table_cache->free_sampler;
    cgpu_assert(fn_free_sampler && "free_sampler Proc Missing!");
    fn_free_sampler(sampler);
}

CGPUTextureViewId cgpu_create_texture_view(CGPUDeviceId device, const struct CGPUTextureViewDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_texture_view && "create_texture_view Proc Missing!");
    CGPUTextureViewDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(CGPUTextureViewDescriptor));
    if (desc->array_layer_count == 0) new_desc.array_layer_count = 1;
    if (desc->mip_level_count == 0) new_desc.mip_level_count = 1;
    CGPUProcCreateTextureView fn_create_texture_view = device->proc_table_cache->create_texture_view;
    CGPUTextureView* texture_view = (CGPUTextureView*)fn_create_texture_view(device, &new_desc);
    texture_view->device = device;
    texture_view->info = *desc;
    return texture_view;
}

void cgpu_free_texture_view(CGPUTextureViewId render_target)
{
    cgpu_assert(render_target != CGPU_NULLPTR && "fatal: call on NULL render_target!");
    const CGPUDeviceId device = render_target->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");

    CGPUProcFreeTextureView fn_free_texture_view = device->proc_table_cache->free_texture_view;
    cgpu_assert(fn_free_texture_view && "free_texture_view Proc Missing!");
    fn_free_texture_view(render_target);
}

bool cgpu_try_bind_aliasing_texture(CGPUDeviceId device, const struct CGPUTextureAliasingBindDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    CGPUProcTryBindAliasingTexture fn_try_bind_aliasing = device->proc_table_cache->try_bind_aliasing_texture;
    cgpu_assert(fn_try_bind_aliasing && "try_bind_aliasing_texture Proc Missing!");
    return fn_try_bind_aliasing(device, desc);
}

// Shared Resource APIs
uint64_t cgpu_export_shared_texture_handle(CGPUDeviceId device, const struct CGPUExportTextureDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    CGPUProcExportSharedTextureHandle fn_export_shared_texture = device->proc_table_cache->export_shared_texture_handle;
    if (!fn_export_shared_texture) return UINT64_MAX;
    return fn_export_shared_texture(device, desc);
}

CGPUTextureId cgpu_import_shared_texture_handle(CGPUDeviceId device, const struct CGPUImportTextureDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    CGPUProcImportSharedTextureHandle fn_import_shared_texture = device->proc_table_cache->import_shared_texture_handle;
    if (!fn_import_shared_texture) return CGPU_NULLPTR;
    CGPUTexture* texture = (CGPUTexture*)fn_import_shared_texture(device, desc);
    if (texture)
    {
        texture->device = device;
        texture->unique_id = ((CGPUDevice*)device)->next_texture_id++;
    }
    return texture;
}

// SwapChain APIs
CGPUSwapChainId cgpu_create_swapchain(CGPUDeviceId device, const CGPUSwapChainDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_swapchain && "create_swapchain Proc Missing!");

    if (desc->present_queues == CGPU_NULLPTR)
    {
        cgpu_assert(desc->present_queues_count <= 0 &&
                    "fatal cgpu_create_swapchain: queue array & queue coutn dismatch!");
    }
    else
    {
        cgpu_assert(desc->present_queues_count > 0 &&
                    "fatal cgpu_create_swapchain: queue array & queue count dismatch!");
    }
    CGPUSwapChain* swapchain = (CGPUSwapChain*)device->proc_table_cache->create_swapchain(device, desc);
    cgpu_assert(swapchain && "fatal cgpu_create_swapchain: NULL swapchain id returned from backend.");
    swapchain->device = device;
    cgpu_trace("cgpu_create_swapchain: swapchain(%dx%d) %p created, buffers: [%p, %p], surface: %p", 
        swapchain->back_buffers[0]->width, swapchain->back_buffers[0]->height, swapchain,
        swapchain->back_buffers[0], swapchain->back_buffers[1], desc->surface);

    for (uint32_t i = 0; i < swapchain->buffer_count; i++)
    {
        ((CGPUTexture*)swapchain->back_buffers[i])->unique_id = ((CGPUDevice*)device)->next_texture_id++;
    }

    return swapchain;
}

uint32_t cgpu_acquire_next_image(CGPUSwapChainId swapchain, const struct CGPUAcquireNextDescriptor* desc)
{
    cgpu_assert(swapchain != CGPU_NULLPTR && "fatal: call on NULL swapchain!");
    cgpu_assert(swapchain->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(swapchain->device->proc_table_cache->acquire_next_image && "acquire_next_image Proc Missing!");

    return swapchain->device->proc_table_cache->acquire_next_image(swapchain, desc);
}

void cgpu_free_swapchain(CGPUSwapChainId swapchain)
{
    cgpu_assert(swapchain != CGPU_NULLPTR && "fatal: call on NULL swapchain!");
    cgpu_assert(swapchain->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(swapchain->device->proc_table_cache->create_swapchain && "create_swapchain Proc Missing!");

    cgpu_trace("cgpu_free_swapchain: swapchain(%dx%d) %p freed, buffers:  [%p, %p]", 
        swapchain->back_buffers[0]->width, swapchain->back_buffers[0]->height, swapchain,
        swapchain->back_buffers[0], swapchain->back_buffers[1]);

    swapchain->device->proc_table_cache->free_swapchain(swapchain);
}

// cgpux helpers
CGPUBufferId cgpux_create_mapped_constant_buffer(CGPUDeviceId device,
uint64_t size, const char8_t* name, bool device_local_preferred)
{
    DECLARE_ZERO(CGPUBufferDescriptor, buf_desc)
    buf_desc.descriptors = CGPU_RESOURCE_TYPE_BUFFER;
    buf_desc.size = size;
    buf_desc.name = name;
    const CGPUAdapterDetail* detail = cgpu_query_adapter_detail(device->adapter);
    buf_desc.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
    buf_desc.flags = CGPU_BCF_PERSISTENT_MAP_BIT | CGPU_BCF_HOST_VISIBLE;
    buf_desc.start_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    if (device_local_preferred && detail->support_host_visible_vram)
    {
        buf_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    }
    return cgpu_create_buffer(device, &buf_desc);
}

RUNTIME_API CGPUBufferId cgpux_create_mapped_upload_buffer(CGPUDeviceId device,
uint64_t size, const char8_t* name)
{
    DECLARE_ZERO(CGPUBufferDescriptor, buf_desc)
    buf_desc.descriptors = CGPU_RESOURCE_TYPE_NONE;
    buf_desc.size = size;
    buf_desc.name = name;
    buf_desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
    buf_desc.flags = CGPU_BCF_PERSISTENT_MAP_BIT;
    buf_desc.start_state = CGPU_RESOURCE_STATE_COPY_DEST;
    return cgpu_create_buffer(device, &buf_desc);
}

bool cgpux_adapter_is_nvidia(CGPUAdapterId adapter)
{
    const CGPUAdapterDetail* detail = cgpu_query_adapter_detail(adapter);
    return (detail->vendor_preset.vendor_id == 0x10DE);
}

bool cgpux_adapter_is_amd(CGPUAdapterId adapter)
{
    const CGPUAdapterDetail* detail = cgpu_query_adapter_detail(adapter);
    return (detail->vendor_preset.vendor_id == 0x1002);
}

bool cgpux_adapter_is_intel(CGPUAdapterId adapter)
{
    const CGPUAdapterDetail* detail = cgpu_query_adapter_detail(adapter);
    return (detail->vendor_preset.vendor_id == 0x8086);
}

// surfaces
#if defined(_WIN32) || defined(_WIN64)
CGPUSurfaceId cgpu_surface_from_hwnd(CGPUDeviceId device, HWND window)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    cgpu_assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    cgpu_assert(device->adapter->instance->surfaces_table->from_hwnd != CGPU_NULLPTR && "free_instance Proc Missing!");

    return device->adapter->instance->surfaces_table->from_hwnd(device, window);
}
#elif defined(_MACOS)
CGPUSurfaceId cgpu_surface_from_ns_view(CGPUDeviceId device, CGPUNSView* window)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    cgpu_assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    cgpu_assert(device->adapter->instance->surfaces_table->from_ns_view != CGPU_NULLPTR && "free_instance Proc Missing!");

    return device->adapter->instance->surfaces_table->from_ns_view(device, window);
}
#endif

CGPUSurfaceId cgpu_surface_from_native_view(CGPUDeviceId device, void* view)
{
#ifdef SKR_OS_MACOSX
    return cgpu_surface_from_ns_view(device, (CGPUNSView*)view);
#elif defined(SKR_OS_WINDOWS)
    return cgpu_surface_from_hwnd(device, (HWND)view);
#endif
    return CGPU_NULLPTR;
}

void cgpu_free_surface(CGPUDeviceId device, CGPUSurfaceId surface)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    cgpu_assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    cgpu_assert(device->adapter->instance->surfaces_table->free_surface != CGPU_NULLPTR && "free_instance Proc Missing!");

    device->adapter->instance->surfaces_table->free_surface(device, surface);
    return;
}

// dstraoge
ECGPUDStorageAvailability cgpu_query_dstorage_availability(CGPUDeviceId device)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    if (device->proc_table_cache->query_dstorage_availability == NULL)
    {
        return CGPU_DSTORAGE_AVAILABILITY_NONE;
    }
    cgpu_assert(device->proc_table_cache->query_dstorage_availability && "query_dstorage_availability Proc Missing!");
    return device->proc_table_cache->query_dstorage_availability(device);
}

CGPUDStorageQueueId cgpu_create_dstorage_queue(CGPUDeviceId device, const CGPUDStorageQueueDescriptor* descriptor)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_dstorage_queue && "create_dstorage_queue Proc Missing!");

    return device->proc_table_cache->create_dstorage_queue(device, descriptor);    
}

CGPUDStorageFileHandle cgpu_dstorage_open_file(CGPUDStorageQueueId queue, const char* abs_path)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->dstorage_open_file && "dstorage_open_file Proc Missing!");

    return queue->device->proc_table_cache->dstorage_open_file(queue, abs_path);    
}

void cgpu_dstorage_query_file_info(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file, CGPUDStorageFileInfo* info)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->dstorage_query_file_info && "dstorage_query_file_info Proc Missing!");

    queue->device->proc_table_cache->dstorage_query_file_info(queue, file, info);
}

void cgpu_dstorage_enqueue_buffer_request(CGPUDStorageQueueId queue, const CGPUDStorageBufferIODescriptor* desc)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->dstorage_enqueue_buffer_request && "dstorage_enqueue_buffer_request Proc Missing!");

    queue->device->proc_table_cache->dstorage_enqueue_buffer_request(queue, desc);
}

void cgpu_dstorage_enqueue_texture_request(CGPUDStorageQueueId queue, const CGPUDStorageTextureIODescriptor* desc)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->dstorage_enqueue_texture_request && "dstorage_enqueue_texture_request Proc Missing!");

    queue->device->proc_table_cache->dstorage_enqueue_texture_request(queue, desc);
}

void cgpu_dstorage_queue_submit(CGPUDStorageQueueId queue, CGPUFenceId fence)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->dstorage_queue_submit && "dstorage_queue_submit Proc Missing!");

    queue->device->proc_table_cache->dstorage_queue_submit(queue, fence);
}

void cgpu_dstorage_close_file(CGPUDStorageQueueId queue, CGPUDStorageFileHandle file)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->dstorage_close_file && "dstorage_close_file Proc Missing!");

    queue->device->proc_table_cache->dstorage_close_file(queue, file);
}

void cgpu_free_dstorage_queue(CGPUDStorageQueueId queue)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->create_dstorage_queue && "create_dstorage_queue Proc Missing!");

    queue->device->proc_table_cache->free_dstorage_queue(queue);    
}

CGPULinkedShaderId cgpu_compile_and_link_shaders(CGPURootSignatureId signature, const struct CGPUCompiledShaderDescriptor* descs, uint32_t count)
{
    cgpu_assert(signature != CGPU_NULLPTR && "fatal: call on NULL signature!");
    cgpu_assert(signature->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(signature->device->proc_table_cache->create_root_signature && "compile_and_link_shaders Proc Missing!");
    CGPULinkedShader* linked = (CGPULinkedShader*)signature->device->proc_table_cache->compile_and_link_shaders(signature, descs, count);
    linked->device = signature->device;
    linked->root_signature = signature;
    return linked;
}

RUNTIME_API void cgpu_compile_shaders(CGPURootSignatureId signature, const struct CGPUCompiledShaderDescriptor* descs, uint32_t count, CGPUCompiledShaderId* out_isas)
{
    cgpu_assert(signature != CGPU_NULLPTR && "fatal: call on NULL signature!");
    cgpu_assert(signature->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(signature->device->proc_table_cache->compile_shaders && "compile_shaders Proc Missing!");
    signature->device->proc_table_cache->compile_shaders(signature, descs, count, out_isas);
}

void cgpu_free_compiled_shader(CGPUCompiledShaderId shader)
{
    cgpu_assert(shader != CGPU_NULLPTR && "fatal: call on NULL shader!");
    const CGPUDeviceId device = shader->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_compiled_shader && "free_compiled_shader Proc Missing!");
    device->proc_table_cache->free_compiled_shader(shader);
}

void cgpu_free_linked_shader(CGPULinkedShaderId shader)
{
    cgpu_assert(shader != CGPU_NULLPTR && "fatal: call on NULL shader!");
    const CGPUDeviceId device = shader->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_linked_shader && "free_linked_shader Proc Missing!");
    device->proc_table_cache->free_linked_shader(shader);
}

CGPUStateStreamId cgpu_create_state_stream(CGPUCommandBufferId cmd, const struct CGPUStateStreamDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(cmd->device->proc_table_cache->create_state_stream && "create_state_stream Proc Missing!");
    CGPUStateStream* stream = (CGPUStateStream*)cmd->device->proc_table_cache->create_state_stream(cmd, desc);
    stream->device = cmd->device;
    stream->cmd = cmd;
    return stream;
}

void cgpu_render_encoder_bind_state_stream(CGPURenderPassEncoderId encoder, CGPUStateStreamId stream)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->render_encoder_bind_state_stream && "render_encoder_bind_state_stream Proc Missing!");
    encoder->device->proc_table_cache->render_encoder_bind_state_stream(encoder, stream);
}

void cgpu_compute_encoder_bind_state_stream(CGPUComputePassEncoderId encoder, CGPUStateStreamId stream)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->compute_encoder_bind_state_stream && "compute_encoder_bind_state_stream Proc Missing!");
    encoder->device->proc_table_cache->compute_encoder_bind_state_stream(encoder, stream);
}

void cgpu_free_state_stream(CGPUStateStreamId stream)
{
    cgpu_assert(stream != CGPU_NULLPTR && "fatal: call on NULL stream!");
    cgpu_assert(stream->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(stream->device->proc_table_cache->free_state_stream && "free_state_stream Proc Missing!");
    stream->device->proc_table_cache->free_state_stream(stream);
}

CGPURasterStateEncoderId cgpu_open_raster_state_encoder(CGPUStateStreamId stream, CGPURenderPassEncoderId rencoder)
{
    cgpu_assert(stream != CGPU_NULLPTR && "fatal: call on NULL stream!");
    cgpu_assert(stream->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(stream->device->proc_table_cache->open_raster_state_encoder && "open_raster_state_encoder Proc Missing!");
    CGPURasterStateEncoder* encoder = (CGPURasterStateEncoder*)stream->device->proc_table_cache->open_raster_state_encoder(stream, rencoder);
    encoder->device = stream->device;
    return encoder;
}

void cgpu_raster_state_encoder_set_viewport(CGPURasterStateEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->raster_state_encoder_set_viewport && "raster_state_encoder_set_viewport Proc Missing!");
    encoder->device->proc_table_cache->raster_state_encoder_set_viewport(encoder, x, y, width, height, min_depth, max_depth);
}

void cgpu_raster_state_encoder_set_scissor(CGPURasterStateEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->raster_state_encoder_set_scissor && "raster_state_encoder_set_scissor Proc Missing!");
    encoder->device->proc_table_cache->raster_state_encoder_set_scissor(encoder, x, y, width, height);
}

void cgpu_raster_state_encoder_set_cull_mode(CGPURasterStateEncoderId encoder, ECGPUCullMode cull_mode)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->raster_state_encoder_set_cull_mode && "raster_state_encoder_set_cull_mode Proc Missing!");
    encoder->device->proc_table_cache->raster_state_encoder_set_cull_mode(encoder, cull_mode);
}

void cgpu_raster_state_encoder_set_front_face(CGPURasterStateEncoderId encoder, ECGPUFrontFace front_face)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->raster_state_encoder_set_front_face && "raster_state_encoder_set_front_face Proc Missing!");
    encoder->device->proc_table_cache->raster_state_encoder_set_front_face(encoder, front_face);
}

void cgpu_raster_state_encoder_set_primitive_topology(CGPURasterStateEncoderId encoder, ECGPUPrimitiveTopology topology)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->raster_state_encoder_set_primitive_topology && "raster_state_encoder_set_primitive_topology Proc Missing!");
    encoder->device->proc_table_cache->raster_state_encoder_set_primitive_topology(encoder, topology);
}

void cgpu_raster_state_encoder_set_depth_test_enabled(CGPURasterStateEncoderId encoder, bool enabled)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->raster_state_encoder_set_depth_test_enabled && "raster_state_encoder_set_depth_test_enabled Proc Missing!");
    encoder->device->proc_table_cache->raster_state_encoder_set_depth_test_enabled(encoder, enabled);
}

void cgpu_raster_state_encoder_set_depth_write_enabled(CGPURasterStateEncoderId encoder, bool enabled)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->raster_state_encoder_set_depth_write_enabled && "raster_state_encoder_set_depth_write_enabled Proc Missing!");
    encoder->device->proc_table_cache->raster_state_encoder_set_depth_write_enabled(encoder, enabled);
}

void cgpu_raster_state_encoder_set_depth_compare_op(CGPURasterStateEncoderId encoder, ECGPUCompareMode function)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->raster_state_encoder_set_depth_compare_op && "raster_state_encoder_set_depth_compare_op Proc Missing!");
    encoder->device->proc_table_cache->raster_state_encoder_set_depth_compare_op(encoder, function);
}

void cgpu_raster_state_encoder_set_stencil_test_enabled(CGPURasterStateEncoderId encoder, bool enabled)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->raster_state_encoder_set_stencil_test_enabled && "raster_state_encoder_set_stencil_test_enabled Proc Missing!");
    encoder->device->proc_table_cache->raster_state_encoder_set_stencil_test_enabled(encoder, enabled);
}

void cgpu_raster_state_encoder_set_stencil_compare_op(CGPURasterStateEncoderId encoder, CGPUStencilFaces faces, ECGPUStencilOp failOp, ECGPUStencilOp passOp, ECGPUStencilOp depthFailOp, ECGPUCompareMode compareOp)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->raster_state_encoder_set_stencil_compare_op && "raster_state_encoder_set_stencil_compare_op Proc Missing!");
    encoder->device->proc_table_cache->raster_state_encoder_set_stencil_compare_op(encoder, faces, failOp, passOp, depthFailOp, compareOp);
}

void cgpu_raster_state_encoder_set_fill_mode(CGPURasterStateEncoderId encoder, ECGPUFillMode fill_mode)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->raster_state_encoder_set_fill_mode && "raster_state_encoder_set_fill_mode Proc Missing!");
    encoder->device->proc_table_cache->raster_state_encoder_set_fill_mode(encoder, fill_mode);
}

void cgpu_raster_state_encoder_set_sample_count(CGPURasterStateEncoderId encoder, ECGPUSampleCount sample_count)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->raster_state_encoder_set_sample_count && "raster_state_encoder_set_sample_count Proc Missing!");
    encoder->device->proc_table_cache->raster_state_encoder_set_sample_count(encoder, sample_count);
}

void cgpu_close_raster_state_encoder(CGPURasterStateEncoderId encoder)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->close_raster_state_encoder && "close_raster_state_encoder Proc Missing!");
    encoder->device->proc_table_cache->close_raster_state_encoder(encoder);
}

CGPUShaderStateEncoderId cgpu_open_shader_state_encoder_r(CGPUStateStreamId stream, CGPURenderPassEncoderId encoder)
{
    cgpu_assert(stream != CGPU_NULLPTR && "fatal: call on NULL stream!");
    cgpu_assert(stream->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(stream->device->proc_table_cache->open_shader_state_encoder_r && "open_shader_state_encoder_r Proc Missing!");
    return stream->device->proc_table_cache->open_shader_state_encoder_r(stream, encoder);
}

CGPUShaderStateEncoderId cgpu_open_shader_state_encoder_c(CGPUStateStreamId stream, CGPUComputePassEncoderId encoder)
{
    cgpu_assert(stream != CGPU_NULLPTR && "fatal: call on NULL stream!");
    cgpu_assert(stream->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(stream->device->proc_table_cache->open_shader_state_encoder_c && "open_shader_state_encoder_c Proc Missing!");
    return stream->device->proc_table_cache->open_shader_state_encoder_c(stream, encoder);
}

void cgpu_shader_state_encoder_bind_shaders(CGPUShaderStateEncoderId encoder, uint32_t stage_count, const ECGPUShaderStage* stages, const CGPUCompiledShaderId* shaders)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->shader_state_encoder_bind_shaders && "shader_state_encoder_bind_shaders Proc Missing!");
    encoder->device->proc_table_cache->shader_state_encoder_bind_shaders(encoder, stage_count, stages, shaders);
}

void cgpu_shader_state_encoder_bind_linked_shader(CGPUShaderStateEncoderId encoder, CGPULinkedShaderId linked)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->shader_state_encoder_bind_linked_shader && "shader_state_encoder_bind_linked_shader Proc Missing!");
    encoder->device->proc_table_cache->shader_state_encoder_bind_linked_shader(encoder, linked);
}

void cgpu_close_shader_state_encoder(CGPUShaderStateEncoderId encoder)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->close_shader_state_encoder && "close_shader_state_encoder Proc Missing!");
    encoder->device->proc_table_cache->close_shader_state_encoder(encoder);
}

CGPUUserStateEncoderId cgpu_open_user_state_encoder(CGPUStateStreamId stream, CGPURenderPassEncoderId encoder)
{
    cgpu_assert(stream != CGPU_NULLPTR && "fatal: call on NULL stream!");
    cgpu_assert(stream->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(stream->device->proc_table_cache->open_user_state_encoder && "open_user_state_encoder Proc Missing!");
    return stream->device->proc_table_cache->open_user_state_encoder(stream, encoder);
}

void cgpu_close_user_state_encoder(CGPUUserStateEncoderId encoder)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL encoder!");
    cgpu_assert(encoder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(encoder->device->proc_table_cache->close_user_state_encoder && "close_user_state_encoder Proc Missing!");
    encoder->device->proc_table_cache->close_user_state_encoder(encoder);
}

CGPUBinderId cgpu_create_binder(CGPUCommandBufferId cmd)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmd!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(cmd->device->proc_table_cache->create_binder && "create_binder Proc Missing!");
    CGPUBinder* binder = (CGPUBinder*)cmd->device->proc_table_cache->create_binder(cmd);
    binder->device = cmd->device;
    binder->cmd = cmd;
    return binder;
}

void cgpu_binder_bind_vertex_layout(CGPUBinderId binder, const struct CGPUVertexLayout* layout)
{
    cgpu_assert(binder != CGPU_NULLPTR && "fatal: call on NULL binder!");
    cgpu_assert(binder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(binder->device->proc_table_cache->binder_bind_vertex_layout && "binder_bind_vertex_layout Proc Missing!");
    binder->device->proc_table_cache->binder_bind_vertex_layout(binder, layout);
}

void cgpu_binder_bind_vertex_buffer(CGPUBinderId binder, uint32_t first_binding, uint32_t binding_count, const CGPUBufferId* buffers, const uint64_t* offsets, const uint64_t* sizes, const uint64_t* strides)
{
    cgpu_assert(binder != CGPU_NULLPTR && "fatal: call on NULL binder!");
    cgpu_assert(binder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(binder->device->proc_table_cache->binder_bind_vertex_buffer && "binder_bind_vertex_buffer Proc Missing!");
    binder->device->proc_table_cache->binder_bind_vertex_buffer(binder, first_binding, binding_count, buffers, offsets, sizes, strides);
}

void cgpu_free_binder(CGPUBinderId binder)
{
    cgpu_assert(binder != CGPU_NULLPTR && "fatal: call on NULL binder!");
    cgpu_assert(binder->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(binder->device->proc_table_cache->free_binder && "free_binder Proc Missing!");
    binder->device->proc_table_cache->free_binder(binder);
}