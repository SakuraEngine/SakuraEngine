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

CGpuInstanceId cgpu_create_instance(const CGpuInstanceDescriptor* desc)
{
    cgpu_assert((desc->backend == ECGpuBackend_VULKAN || desc->backend == ECGpuBackend_D3D12 || desc->backend == ECGpuBackend_METAL) && "cgpu support only vulkan & d3d12 currently!");
    const CGpuProcTable* tbl = CGPU_NULLPTR;
    const CGpuSurfacesProcTable* s_tbl = CGPU_NULLPTR;

    if (desc->backend == ECGpuBackend_COUNT)
    {
    }
#ifdef CGPU_USE_VULKAN
    else if (desc->backend == ECGpuBackend_VULKAN)
    {
        tbl = CGPU_VulkanProcTable();
        s_tbl = CGPU_VulkanSurfacesProcTable();
    }
#endif
#ifdef CGPU_USE_METAL
    else if (desc->backend == ECGpuBackend_METAL)
    {
        tbl = CGPU_MetalProcTable();
        s_tbl = CGPU_MetalSurfacesProcTable();
    }
#endif
#ifdef CGPU_USE_D3D12
    else if (desc->backend == ECGpuBackend_D3D12)
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
    cgpu_assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    cgpu_assert(instance->proc_table->query_instance_features && "query_instance_features Proc Missing!");

    instance->proc_table->query_instance_features(instance, features);
}

void cgpu_free_instance(CGpuInstanceId instance)
{
    cgpu_assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    cgpu_assert(instance->proc_table->free_instance && "free_instance Proc Missing!");

    cgpu_free_runtime_table(instance->runtime_table);
    instance->proc_table->free_instance(instance);
}

void cgpu_enum_adapters(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num)
{
    cgpu_assert(instance != CGPU_NULLPTR && "fatal: can't destroy NULL instance!");
    cgpu_assert(instance->proc_table->enum_adapters && "enum_adapters Proc Missing!");

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
    cgpu_assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(adapter->proc_table_cache->query_adapter_detail && "query_adapter_detail Proc Missing!");

    CGpuAdapterDetail* detail = (CGpuAdapterDetail*)adapter->proc_table_cache->query_adapter_detail(adapter);
    return detail;
}

uint32_t cgpu_query_queue_count(const CGpuAdapterId adapter, const ECGpuQueueType type)
{
    cgpu_assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(adapter->proc_table_cache->query_queue_count && "query_queue_count Proc Missing!");

    return adapter->proc_table_cache->query_queue_count(adapter, type);
}

CGpuDeviceId cgpu_create_device(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc)
{
    cgpu_assert(adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(adapter->proc_table_cache->create_device && "create_device Proc Missing!");

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
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_fence && "create_fence Proc Missing!");
    CGpuFence* fence = (CGpuFence*)device->proc_table_cache->create_fence(device);
    fence->device = device;
    return fence;
}

void cgpu_wait_fences(const CGpuFenceId* fences, uint32_t fence_count)
{
    if (fences == CGPU_NULLPTR || fence_count <= 0)
    {
        return;
    }
    CGpuFenceId fence = fences[0];
    cgpu_assert(fence != CGPU_NULLPTR && "fatal: call on NULL fence!");
    cgpu_assert(fence->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcWaitFences fn_wait_fences = fence->device->proc_table_cache->wait_fences;
    cgpu_assert(fn_wait_fences && "wait_fences Proc Missing!");
    fn_wait_fences(fences, fence_count);
}

void cgpu_free_fence(CGpuFenceId fence)
{
    cgpu_assert(fence != CGPU_NULLPTR && "fatal: call on NULL fence!");
    cgpu_assert(fence->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcFreeFence fn_free_fence = fence->device->proc_table_cache->free_fence;
    cgpu_assert(fn_free_fence && "free_fence Proc Missing!");
    fn_free_fence(fence);
}

CGpuRootSignatureId cgpu_create_root_signature(CGpuDeviceId device, const struct CGpuRootSignatureDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_root_signature && "create_root_signature Proc Missing!");
    CGpuRootSignature* signature = (CGpuRootSignature*)device->proc_table_cache->create_root_signature(device, desc);
    signature->device = device;
    return signature;
}

void cgpu_free_root_signature(CGpuRootSignatureId signature)
{
    cgpu_assert(signature != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGpuDeviceId device = signature->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_root_signature && "free_root_signature Proc Missing!");
    device->proc_table_cache->free_root_signature(signature);
}

CGpuDescriptorSetId cgpu_create_descriptor_set(CGpuDeviceId device, const struct CGpuDescriptorSetDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_descriptor_set && "create_descriptor_set Proc Missing!");
    CGpuDescriptorSet* set = (CGpuDescriptorSet*)device->proc_table_cache->create_descriptor_set(device, desc);
    set->root_signature = desc->root_signature;
    set->index = desc->set_index;
    return set;
}

void cgpu_update_descriptor_set(CGpuDescriptorSetId set, const struct CGpuDescriptorData* datas, uint32_t count)
{
    cgpu_assert(set != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGpuDeviceId device = set->root_signature->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->update_descriptor_set && "update_descriptor_set Proc Missing!");
    device->proc_table_cache->update_descriptor_set(set, datas, count);
}

void cgpu_free_descriptor_set(CGpuDescriptorSetId set)
{
    cgpu_assert(set != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGpuDeviceId device = set->root_signature->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_descriptor_set && "free_descriptor_set Proc Missing!");
    device->proc_table_cache->free_descriptor_set(set);
}

CGpuComputePipelineId cgpu_create_compute_pipeline(CGpuDeviceId device, const struct CGpuComputePipelineDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_compute_pipeline && "create_compute_pipeline Proc Missing!");
    CGpuComputePipeline* pipeline = (CGpuComputePipeline*)device->proc_table_cache->create_compute_pipeline(device, desc);
    pipeline->device = device;
    return pipeline;
}

void cgpu_free_compute_pipeline(CGpuComputePipelineId pipeline)
{
    cgpu_assert(pipeline != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGpuDeviceId device = pipeline->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_compute_pipeline && "free_compute_pipeline Proc Missing!");
    device->proc_table_cache->free_compute_pipeline(pipeline);
}

CGpuRenderPipelineId cgpu_create_render_pipeline(CGpuDeviceId device, const struct CGpuRenderPipelineDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_render_pipeline && "create_render_pipeline Proc Missing!");
    CGpuRenderPipeline* pipeline = CGPU_NULLPTR;
    if (desc->blend_state == CGPU_NULLPTR)
    {
        CGpuBlendStateDescriptor blendStateDesc = {
            .src_factors[0] = BC_ONE,
            .dst_factors[0] = BC_ZERO,
            .blend_modes[0] = BM_ADD,
            .src_alpha_factors[0] = BC_ONE,
            .dst_alpha_factors[0] = BC_ZERO,
            .masks[0] = COLOR_MASK_ALL,
            .independent_blend = false
        };
        ((CGpuRenderPipelineDescriptor*)desc)->blend_state = &blendStateDesc;
        pipeline = (CGpuRenderPipeline*)device->proc_table_cache->create_render_pipeline(device, desc);
    }
    else
    {
        pipeline = (CGpuRenderPipeline*)device->proc_table_cache->create_render_pipeline(device, desc);
    }
    pipeline->device = device;
    return pipeline;
}

void cgpu_free_render_pipeline(CGpuRenderPipelineId pipeline)
{
    cgpu_assert(pipeline != CGPU_NULLPTR && "fatal: call on NULL signature!");
    const CGpuDeviceId device = pipeline->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_render_pipeline && "free_render_pipeline Proc Missing!");
    device->proc_table_cache->free_render_pipeline(pipeline);
}

void cgpu_free_device(CGpuDeviceId device)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_device && "free_device Proc Missing!");

    device->proc_table_cache->free_device(device);
    return;
}

CGpuQueueId cgpu_get_queue(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->free_device && "free_device Proc Missing!");

    CGpuQueueId created = cgpu_runtime_table_try_get_queue(device, type, index);
    if (created != NULL)
    {
        cgpu_warn("You should not call cgpu_get_queue "
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
    cgpu_assert(desc != CGPU_NULLPTR && "fatal: call on NULL desc!");
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcSubmitQueue submit_queue = queue->device->proc_table_cache->submit_queue;
    cgpu_assert(submit_queue && "submit_queue Proc Missing!");

    submit_queue(queue, desc);
}

void cgpu_queue_present(CGpuQueueId queue, const struct CGpuQueuePresentDescriptor* desc)
{
    cgpu_assert(desc != CGPU_NULLPTR && "fatal: call on NULL desc!");
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcQueuePresent fn_queue_present = queue->device->proc_table_cache->queue_present;
    cgpu_assert(fn_queue_present && "queue_present Proc Missing!");

    fn_queue_present(queue, desc);
}

void cgpu_wait_queue_idle(CGpuQueueId queue)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcWaitQueueIdle wait_queue_idle = queue->device->proc_table_cache->wait_queue_idle;
    cgpu_assert(wait_queue_idle && "wait_queue_idle Proc Missing!");

    wait_queue_idle(queue);
}

void cgpu_free_queue(CGpuQueueId queue)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->free_queue && "free_queue Proc Missing!");

    queue->device->proc_table_cache->free_queue(queue);
    return;
}

RUNTIME_API CGpuCommandPoolId cgpu_create_command_pool(CGpuQueueId queue,
    const CGpuCommandPoolDescriptor* desc)
{
    cgpu_assert(queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(queue->device->proc_table_cache->create_command_pool && "create_command_pool Proc Missing!");

    CGpuCommandPool* pool = (CGpuCommandPool*)queue->device->proc_table_cache->create_command_pool(queue, desc);
    pool->queue = queue;
    return pool;
}

RUNTIME_API CGpuCommandBufferId cgpu_create_command_buffer(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    const CGpuDeviceId device = pool->queue->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCreateCommandBuffer fn_create_cmd = device->proc_table_cache->create_command_buffer;
    cgpu_assert(fn_create_cmd && "create_command_buffer Proc Missing!");

    CGpuCommandBuffer* cmd = (CGpuCommandBuffer*)fn_create_cmd(pool, desc);
    cmd->pool = pool;
    cmd->device = device;
    return cmd;
}

RUNTIME_API void cgpu_reset_command_pool(CGpuCommandPoolId pool)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(pool->queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(pool->queue->device->proc_table_cache->reset_command_pool && "reset_command_pool Proc Missing!");

    pool->queue->device->proc_table_cache->reset_command_pool(pool);
    return;
}

RUNTIME_API void cgpu_free_command_buffer(CGpuCommandBufferId cmd)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    CGpuCommandPoolId pool = cmd->pool;
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    const CGpuDeviceId device = pool->queue->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcFreeCommandBuffer fn_free_cmd = device->proc_table_cache->free_command_buffer;
    cgpu_assert(fn_free_cmd && "free_command_buffer Proc Missing!");

    fn_free_cmd(cmd);
}

RUNTIME_API void cgpu_free_command_pool(CGpuCommandPoolId pool)
{
    cgpu_assert(pool != CGPU_NULLPTR && "fatal: call on NULL pool!");
    cgpu_assert(pool->queue != CGPU_NULLPTR && "fatal: call on NULL queue!");
    cgpu_assert(pool->queue->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(pool->queue->device->proc_table_cache->free_command_pool && "free_command_pool Proc Missing!");

    pool->queue->device->proc_table_cache->free_command_pool(pool);
    return;
}

// CMDs
void cgpu_cmd_begin(CGpuCommandBufferId cmd)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBegin fn_cmd_begin = cmd->device->proc_table_cache->cmd_begin;
    cgpu_assert(fn_cmd_begin && "cmd_begin Proc Missing!");
    fn_cmd_begin(cmd);
}

void cgpu_cmd_update_buffer(CGpuCommandBufferId cmd, const struct CGpuBufferUpdateDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->current_dispatch == PT_NONE && "fatal: can't call transfer apis on commdn buffer while preparing dispatching!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdUpdateBuffer fn_cmd_update_buffer = cmd->device->proc_table_cache->cmd_update_buffer;
    cgpu_assert(fn_cmd_update_buffer && "cmd_update_buffer Proc Missing!");
    fn_cmd_update_buffer(cmd, desc);
}

void cgpu_cmd_resource_barrier(CGpuCommandBufferId cmd, const struct CGpuResourceBarrierDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->current_dispatch == PT_NONE && "fatal: can't call resource barriers in render/dispatch passes!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdResourceBarrier fn_cmd_resource_barrier = cmd->device->proc_table_cache->cmd_resource_barrier;
    cgpu_assert(fn_cmd_resource_barrier && "cmd_resource_barrier Proc Missing!");
    fn_cmd_resource_barrier(cmd, desc);
}

void cgpu_cmd_end(CGpuCommandBufferId cmd)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdEnd fn_cmd_end = cmd->device->proc_table_cache->cmd_end;
    cgpu_assert(fn_cmd_end && "cmd_end Proc Missing!");
    fn_cmd_end(cmd);
}

// Compute CMDs
CGpuComputePassEncoderId cgpu_cmd_begin_compute_pass(CGpuCommandBufferId cmd, const struct CGpuComputePassDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBeginComputePass fn_begin_compute_pass = cmd->device->proc_table_cache->cmd_begin_compute_pass;
    cgpu_assert(fn_begin_compute_pass && "cmd_begin_compute_pass Proc Missing!");
    CGpuComputePassEncoderId ecd = (CGpuComputePassEncoderId)fn_begin_compute_pass(cmd, desc);
    CGpuCommandBuffer* Cmd = (CGpuCommandBuffer*)cmd;
    Cmd->current_dispatch = PT_COMPUTE;
    return ecd;
}

void cgpu_compute_encoder_bind_descriptor_set(CGpuComputePassEncoderId encoder, CGpuDescriptorSetId set)
{
    cgpu_assert(encoder != CGPU_NULLPTR && "fatal: call on NULL compute encoder!");
    cgpu_assert(set != CGPU_NULLPTR && "fatal: call on NULL descriptor!");
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcComputeEncoderBindDescriptorSet fn_bind_descriptor_set = device->proc_table_cache->compute_encoder_bind_descriptor_set;
    cgpu_assert(fn_bind_descriptor_set && "compute_encoder_bind_descriptor_set Proc Missing!");
    fn_bind_descriptor_set(encoder, set);
}

void cgpu_compute_encoder_bind_pipeline(CGpuComputePassEncoderId encoder, CGpuComputePipelineId pipeline)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcComputeEncoderBindPipeline fn_compute_bind_pipeline = device->proc_table_cache->compute_encoder_bind_pipeline;
    cgpu_assert(fn_compute_bind_pipeline && "compute_encoder_bind_pipeline Proc Missing!");
    fn_compute_bind_pipeline(encoder, pipeline);
}

void cgpu_compute_encoder_dispatch(CGpuComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcComputeEncoderDispatch fn_compute_dispatch = device->proc_table_cache->compute_encoder_dispatch;
    cgpu_assert(fn_compute_dispatch && "compute_encoder_dispatch Proc Missing!");
    fn_compute_dispatch(encoder, X, Y, Z);
}

void cgpu_cmd_end_compute_pass(CGpuCommandBufferId cmd, CGpuComputePassEncoderId encoder)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(cmd->current_dispatch == PT_COMPUTE && "fatal: can't call end command pass on commnd buffer while not dispatching compute!");
    const CGPUProcCmdEndComputePass fn_end_compute_pass = cmd->device->proc_table_cache->cmd_end_compute_pass;
    cgpu_assert(fn_end_compute_pass && "cmd_end_compute_pass Proc Missing!");
    fn_end_compute_pass(cmd, encoder);
    CGpuCommandBuffer* Cmd = (CGpuCommandBuffer*)cmd;
    Cmd->current_dispatch = PT_NONE;
}

// Render CMDs
CGpuRenderPassEncoderId cgpu_cmd_begin_render_pass(CGpuCommandBufferId cmd, const struct CGpuRenderPassDescriptor* desc)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcCmdBeginRenderPass fn_begin_render_pass = cmd->device->proc_table_cache->cmd_begin_render_pass;
    cgpu_assert(fn_begin_render_pass && "cmd_begin_render_pass Proc Missing!");
    CGpuRenderPassEncoderId ecd = (CGpuRenderPassEncoderId)fn_begin_render_pass(cmd, desc);
    CGpuCommandBuffer* Cmd = (CGpuCommandBuffer*)cmd;
    Cmd->current_dispatch = PT_GRAPHICS;
    return ecd;
}

void cgpu_render_encoder_set_viewport(CGpuRenderPassEncoderId encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderSetViewport fn_render_set_viewport = device->proc_table_cache->render_encoder_set_viewport;
    cgpu_assert(fn_render_set_viewport && "render_encoder_set_viewport Proc Missing!");
    fn_render_set_viewport(encoder, x, y, width, height, min_depth, max_depth);
}

void cgpu_render_encoder_set_scissor(CGpuRenderPassEncoderId encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderSetScissor fn_render_set_scissor = device->proc_table_cache->render_encoder_set_scissor;
    cgpu_assert(fn_render_set_scissor && "render_encoder_set_scissor Proc Missing!");
    fn_render_set_scissor(encoder, x, y, width, height);
}

void cgpu_render_encoder_bind_pipeline(CGpuRenderPassEncoderId encoder, CGpuRenderPipelineId pipeline)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderBindPipeline fn_render_bind_pipeline = device->proc_table_cache->render_encoder_bind_pipeline;
    cgpu_assert(fn_render_bind_pipeline && "render_encoder_bind_pipeline Proc Missing!");
    fn_render_bind_pipeline(encoder, pipeline);
}

void cgpu_render_encoder_draw(CGpuRenderPassEncoderId encoder, uint32_t vertex_count, uint32_t first_vertex)
{
    CGpuDeviceId device = encoder->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    const CGPUProcRenderEncoderDraw fn_draw = device->proc_table_cache->render_encoder_draw;
    cgpu_assert(fn_draw && "render_encoder_draw Proc Missing!");
    fn_draw(encoder, vertex_count, first_vertex);
}

void cgpu_cmd_end_render_pass(CGpuCommandBufferId cmd, CGpuRenderPassEncoderId encoder)
{
    cgpu_assert(cmd != CGPU_NULLPTR && "fatal: call on NULL cmdbuffer!");
    cgpu_assert(cmd->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(cmd->current_dispatch == PT_GRAPHICS && "fatal: can't call end command pass on commnd buffer while not dispatching graphics!");
    const CGPUProcCmdEndRenderPass fn_end_render_pass = cmd->device->proc_table_cache->cmd_end_render_pass;
    cgpu_assert(fn_end_render_pass && "cmd_end_render_pass Proc Missing!");
    fn_end_render_pass(cmd, encoder);
    CGpuCommandBuffer* Cmd = (CGpuCommandBuffer*)cmd;
    Cmd->current_dispatch = PT_NONE;
}

// Shader APIs
CGpuShaderLibraryId cgpu_create_shader_library(CGpuDeviceId device, const struct CGpuShaderLibraryDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_shader_library && "create_shader_library Proc Missing!");

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
    cgpu_assert(library != CGPU_NULLPTR && "fatal: call on NULL shader library!");
    const CGpuDeviceId device = library->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    // handle name string
    cgpu_free((void*)library->name);

    CGPUProcFreeShaderLibrary fn_free_shader_library = device->proc_table_cache->free_shader_library;
    cgpu_assert(fn_free_shader_library && "free_shader_library Proc Missing!");
    fn_free_shader_library(library);
}

// Buffer APIs
CGpuBufferId cgpu_create_buffer(CGpuDeviceId device, const struct CGpuBufferDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_buffer && "create_buffer Proc Missing!");
    if (desc->flags == 0)
    {
        CGpuBufferDescriptor* wdesc = (CGpuBufferDescriptor*)desc;
        wdesc->flags |= BCF_NONE;
    }
    CGPUProcCreateBuffer fn_create_buffer = device->proc_table_cache->create_buffer;
    CGpuBuffer* buffer = (CGpuBuffer*)fn_create_buffer(device, desc);
    buffer->device = device;
    return buffer;
}

void cgpu_map_buffer(CGpuBufferId buffer, const struct CGpuBufferRange* range)
{
    cgpu_assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    const CGpuDeviceId device = buffer->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->map_buffer && "map_buffer Proc Missing!");

    CGPUProcMapBuffer fn_map_buffer = device->proc_table_cache->map_buffer;
    fn_map_buffer(buffer, range);
}

void cgpu_unmap_buffer(CGpuBufferId buffer)
{
    cgpu_assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    const CGpuDeviceId device = buffer->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->unmap_buffer && "unmap_buffer Proc Missing!");

    CGPUProcUnmapBuffer fn_unmap_buffer = device->proc_table_cache->unmap_buffer;
    fn_unmap_buffer(buffer);
}

void cgpu_free_buffer(CGpuBufferId buffer)
{
    cgpu_assert(buffer != CGPU_NULLPTR && "fatal: call on NULL buffer!");
    const CGpuDeviceId device = buffer->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");

    CGPUProcFreeBuffer fn_free_buffer = device->proc_table_cache->free_buffer;
    cgpu_assert(fn_free_buffer && "free_buffer Proc Missing!");
    fn_free_buffer(buffer);
}

// Texture/TextureView APIs
CGpuTextureId cgpu_create_texture(CGpuDeviceId device, const struct CGpuTextureDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_texture && "create_texture Proc Missing!");
    CGpuTextureDescriptor* wdesc = (CGpuTextureDescriptor*)desc;
    if (desc->array_size == 0) wdesc->array_size = 1;
    if (desc->mip_levels == 0) wdesc->mip_levels = 1;
    CGPUProcCreateTexture fn_create_texture = device->proc_table_cache->create_texture;
    CGpuTexture* texture = (CGpuTexture*)fn_create_texture(device, desc);
    texture->device = device;
    return texture;
}

void cgpu_free_texture(CGpuTextureId texture)
{
    cgpu_assert(texture != CGPU_NULLPTR && "fatal: call on NULL texture!");
    const CGpuDeviceId device = texture->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");

    CGPUProcFreeTexture fn_free_texture = device->proc_table_cache->free_texture;
    cgpu_assert(fn_free_texture && "free_texture Proc Missing!");
    fn_free_texture(texture);
}

CGpuTextureViewId cgpu_create_texture_view(CGpuDeviceId device, const struct CGpuTextureViewDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_texture_view && "create_texture_view Proc Missing!");
    CGpuTextureViewDescriptor* wdesc = (CGpuTextureViewDescriptor*)desc;
    if (desc->array_layer_count == 0) wdesc->array_layer_count = 1;
    if (desc->mip_level_count == 0) wdesc->mip_level_count = 1;
    CGPUProcCreateTextureView fn_create_texture_view = device->proc_table_cache->create_texture_view;
    CGpuTextureView* texture_view = (CGpuTextureView*)fn_create_texture_view(device, desc);
    texture_view->device = device;
    texture_view->info = *desc;
    return texture_view;
}

void cgpu_free_texture_view(CGpuTextureViewId render_target)
{
    cgpu_assert(render_target != CGPU_NULLPTR && "fatal: call on NULL render_target!");
    const CGpuDeviceId device = render_target->device;
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");

    CGPUProcFreeTextureView fn_free_texture_view = device->proc_table_cache->free_texture_view;
    cgpu_assert(fn_free_texture_view && "free_texture_view Proc Missing!");
    fn_free_texture_view(render_target);
}

// SwapChain APIs
CGpuSwapChainId cgpu_create_swapchain(CGpuDeviceId device, const CGpuSwapChainDescriptor* desc)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->proc_table_cache->create_swapchain && "create_swapchain Proc Missing!");

    if (desc->presentQueues == CGPU_NULLPTR)
    {
        cgpu_assert(desc->presentQueuesCount <= 0 &&
                    "fatal cgpu_create_swapchain: queue array & queue coutn dismatch!");
    }
    else
    {
        cgpu_assert(desc->presentQueuesCount > 0 &&
                    "fatal cgpu_create_swapchain: queue array & queue coutn dismatch!");
    }
    CGpuSwapChain* swapchain = (CGpuSwapChain*)device->proc_table_cache->create_swapchain(device, desc);
    cgpu_assert(swapchain && "fatal cgpu_create_swapchain: NULL swapchain id returned from backend.");
    swapchain->device = device;
    return swapchain;
}

uint32_t cgpu_acquire_next_image(CGpuSwapChainId swapchain, const struct CGpuAcquireNextDescriptor* desc)
{
    cgpu_assert(swapchain != CGPU_NULLPTR && "fatal: call on NULL swapchain!");
    cgpu_assert(swapchain->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(swapchain->device->proc_table_cache->acquire_next_image && "acquire_next_image Proc Missing!");

    return swapchain->device->proc_table_cache->acquire_next_image(swapchain, desc);
}

void cgpu_free_swapchain(CGpuSwapChainId swapchain)
{
    cgpu_assert(swapchain != CGPU_NULLPTR && "fatal: call on NULL swapchain!");
    cgpu_assert(swapchain->device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(swapchain->device->proc_table_cache->create_swapchain && "create_swapchain Proc Missing!");

    swapchain->device->proc_table_cache->free_swapchain(swapchain);
}

// surfaces
#if defined(_WIN32) || defined(_WIN64)
CGpuSurfaceId cgpu_surface_from_hwnd(CGpuDeviceId device, HWND window)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    cgpu_assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    cgpu_assert(device->adapter->instance->surfaces_table->from_hwnd != CGPU_NULLPTR && "free_instance Proc Missing!");

    return device->adapter->instance->surfaces_table->from_hwnd(device, window);
}
#elif defined(_MACOS)
CGpuSurfaceId cgpu_surface_from_ns_view(CGpuDeviceId device, CGpuNSView* window)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    cgpu_assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    cgpu_assert(device->adapter->instance->surfaces_table->from_ns_view != CGPU_NULLPTR && "free_instance Proc Missing!");

    return device->adapter->instance->surfaces_table->from_ns_view(device, window);
}
#endif

void cgpu_free_surface(CGpuDeviceId device, CGpuSurfaceId surface)
{
    cgpu_assert(device != CGPU_NULLPTR && "fatal: call on NULL device!");
    cgpu_assert(device->adapter != CGPU_NULLPTR && "fatal: call on NULL adapter!");
    cgpu_assert(device->adapter->instance != CGPU_NULLPTR && "fatal: call on NULL instnace!");
    cgpu_assert(device->adapter->instance->surfaces_table != CGPU_NULLPTR && "surfaces_table Missing!");
    cgpu_assert(device->adapter->instance->surfaces_table->free_surface != CGPU_NULLPTR && "free_instance Proc Missing!");

    device->adapter->instance->surfaces_table->free_surface(device, surface);
    return;
}

// Common Utils
void CGpuUtil_InitRSBlackboardAndParamTables(CGpuRootSignature* RS, CGpuUtil_RSBlackboard* bb, const struct CGpuRootSignatureDescriptor* desc)
{
    DECLARE_ZERO_VLA(CGpuShaderReflection*, entry_reflections, desc->shaders_count)
    // Pick shader reflection data
    for (uint32_t i = 0; i < desc->shaders_count; i++)
    {
        const CGpuPipelineShaderDescriptor* shader_desc = &desc->shaders[i];
        // find shader refl
        for (uint32_t j = 0; j < shader_desc->library->entrys_count; j++)
        {
            CGpuShaderReflection* temp_entry_reflcetion = &shader_desc->library->entry_reflections[j];
            if (strcmp(shader_desc->entry, temp_entry_reflcetion->entry_name) == 0)
            {
                entry_reflections[i] = temp_entry_reflcetion;
                break;
            }
        }
        if (entry_reflections[i] == CGPU_NULLPTR)
        {
            entry_reflections[i] = &shader_desc->library->entry_reflections[0];
        }
    }
    // Count Sets and Binding Slots
    bb->pipelineType = PT_NONE;
    for (uint32_t i = 0; i < desc->shaders_count; i++)
    {
        CGpuShaderReflection* reflection = entry_reflections[i];
        for (uint32_t j = 0; j < reflection->shader_resources_count; j++)
        {
            CGpuShaderResource* resource = &reflection->shader_resources[j];
            bb->set_count = bb->set_count > resource->set + 1 ? bb->set_count : resource->set + 1;
            bb->max_binding = bb->max_binding > resource->binding + 1 ? bb->max_binding : resource->binding + 1;
        }
        // Pipeline Type
        if (reflection->stage & SS_COMPUTE)
            bb->pipelineType = PT_COMPUTE;
        else if (reflection->stage & SS_RAYTRACING)
            bb->pipelineType = PT_RAYTRACING;
        else
            bb->pipelineType = PT_GRAPHICS;
    }
    // Collect Shader Resources
    if (bb->set_count * bb->max_binding > 0)
    {
        // Record On Temporal Memory
        bb->sig_reflections = (CGpuShaderResource*)cgpu_calloc(bb->set_count * bb->max_binding, sizeof(CGpuShaderResource));
        bb->valid_bindings = (uint32_t*)cgpu_calloc(bb->set_count, sizeof(uint32_t));
        for (uint32_t i = 0; i < desc->shaders_count; i++)
        {
            CGpuShaderReflection* reflection = entry_reflections[i];
            for (uint32_t j = 0; j < reflection->shader_resources_count; j++)
            {
                CGpuShaderResource* resource = &reflection->shader_resources[j];
                const uint32_t slot = bb->max_binding * resource->set + resource->binding;
                CGpuShaderStages prev_stages = bb->sig_reflections[slot].stages;
                memcpy(&bb->sig_reflections[slot], resource, sizeof(CGpuShaderResource));
                // Merge stage masks
                bb->sig_reflections[slot].stages |= prev_stages;
                bb->valid_bindings[resource->set] =
                    bb->valid_bindings[resource->set] > resource->binding + 1 ? bb->valid_bindings[resource->set] : resource->binding + 1;
            }
        }
    }
    // Initailize CGpuRootSignature::tables
    RS->table_count = bb->set_count;
    if (bb->set_count * bb->max_binding > 0)
    {
        RS->tables = (CGpuParameterTable*)cgpu_calloc(bb->set_count, sizeof(CGpuParameterTable));
        for (uint32_t i_set = 0; i_set < bb->set_count; i_set++)
        {
            CGpuParameterTable* set_to_record = &RS->tables[i_set];
            set_to_record->resources_count = bb->valid_bindings[i_set];
            set_to_record->resources = cgpu_calloc(set_to_record->resources_count, sizeof(CGpuShaderResource));
            for (uint32_t i_binding = 0; i_binding < set_to_record->resources_count; i_binding++)
            {
                CGpuShaderResource* reflSlot = &bb->sig_reflections[i_set * bb->max_binding + i_binding];
                const size_t source_len = strlen(reflSlot->name);
                set_to_record->resources[i_binding].name = (char8_t*)cgpu_malloc(sizeof(char8_t) * (1 + source_len));
#ifdef _WIN32
                strcpy_s((char8_t*)set_to_record->resources[i_binding].name, source_len + 1, reflSlot->name);
#else
                strcpy((char8_t*)set_to_record->resources[i_binding].name, reflSlot->name);
#endif
                set_to_record->resources[i_binding].type = reflSlot->type;
                set_to_record->resources[i_binding].set = reflSlot->set;
                set_to_record->resources[i_binding].binding = reflSlot->binding;
                set_to_record->resources[i_binding].size = reflSlot->size;
                set_to_record->resources[i_binding].stages = reflSlot->stages;
                set_to_record->resources[i_binding].name_hash = reflSlot->name_hash;
            }
        }
    }
}

void CGpuUtil_FreeRSParamTables(CGpuRootSignature* RS)
{
    if (RS->tables != CGPU_NULLPTR)
    {
        for (uint32_t i_set = 0; i_set < RS->table_count; i_set++)
        {
            CGpuParameterTable* param_table = &RS->tables[i_set];
            if (param_table->resources != CGPU_NULLPTR)
            {
                for (uint32_t i_binding = 0; i_binding < param_table->resources_count; i_binding++)
                {
                    CGpuShaderResource* binding_to_free = &param_table->resources[i_binding];
                    if (binding_to_free->name != CGPU_NULLPTR)
                    {
                        cgpu_free((char8_t*)binding_to_free->name);
                    }
                }

                cgpu_free(param_table->resources);
            }
        }
        cgpu_free(RS->tables);
    }
}

void CGpuUtil_FreeRSBlackboard(CGpuUtil_RSBlackboard* bb)
{
    // Free Temporal Memory
    cgpu_free(bb->sig_reflections);
    cgpu_free(bb->valid_bindings);
}