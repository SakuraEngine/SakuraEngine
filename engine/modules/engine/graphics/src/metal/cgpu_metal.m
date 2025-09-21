#include "SkrBase/misc/debug.h"
#include "SkrGraphics/api.h"
#include "metal_utils.h"

const CGPUProcTable tbl_metal = {
    // Instance APIs
    .create_instance = &cgpu_create_instance_metal,
    .query_instance_features = &cgpu_query_instance_features_metal,
    .free_instance = &cgpu_free_instance_metal,

    // Adapter APIs
    .enum_adapters = &cgpu_enum_adapters_metal,
    .query_adapter_detail = &cgpu_query_adapter_detail_metal,
    .query_queue_count = &cgpu_query_queue_count_metal,

    // Device APIs
    .create_device = &cgpu_create_device_metal,
    .query_video_memory_info = &cgpu_query_video_memory_info_metal,
    .query_shared_memory_info = &cgpu_query_shared_memory_info_metal,
    .free_device = &cgpu_free_device_metal,

    // API Objects APIs
    .create_fence = &cgpu_create_fence_metal,
    .query_fence_status = &cgpu_query_fence_status_metal,
    .wait_fences = &cgpu_wait_fences_metal,
    .free_fence = &cgpu_free_fence_metal,
    .create_semaphore = &cgpu_create_semaphore_metal,
    .free_semaphore = &cgpu_free_semaphore_metal,
    .create_root_signature_pool = &cgpu_create_root_signature_pool_metal,
    .free_root_signature_pool = &cgpu_free_root_signature_pool_metal,
    .create_root_signature = &cgpu_create_root_signature_metal,
    .free_root_signature = &cgpu_free_root_signature_metal,
    .create_compute_pipeline = &cgpu_create_compute_pipeline_metal,
    .free_compute_pipeline = &cgpu_free_compute_pipeline_metal,
    .create_query_pool = &cgpu_create_query_pool_metal,
    .free_query_pool = &cgpu_free_query_pool_metal,

    // Descriptor Set/Buffer
    .create_descriptor_set = &cgpu_create_descriptor_set_metal,
    .update_descriptor_set = &cgpu_update_descriptor_set_metal,
    .free_descriptor_set = &cgpu_free_descriptor_set_metal,
    .create_descriptor_buffer = &cgpu_create_descriptor_buffer_metal,
    .update_descriptor_buffer = &cgpu_update_descriptor_buffer_metal,
    .copy_descriptor_buffer = &cgpu_copy_descriptor_buffer_metal,
    .free_descriptor_buffer = &cgpu_free_descriptor_buffer_metal,

    // Queue APIs
    .get_queue = &cgpu_get_queue_metal,
    .submit_queue = &cgpu_submit_queue_metal,
    .wait_queue_idle = &cgpu_wait_queue_idle_metal,
    .queue_present = &cgpu_queue_present_metal,
    .free_queue = &cgpu_free_queue_metal,

    // Command APIs
    .create_command_pool = &cgpu_create_command_pool_metal,
    .reset_command_pool = &cgpu_reset_command_pool_metal,
    .create_command_buffer = &cgpu_create_command_buffer_metal,
    .free_command_buffer = &cgpu_free_command_buffer_metal,
    .free_command_pool = &cgpu_free_command_pool_metal,

    // Shader APIs
    .create_shader_library = &cgpu_create_shader_library_metal,
    .free_shader_library = &cgpu_free_shader_library_metal,

    // Buffer APIs
    .create_buffer = &cgpu_create_buffer_metal,
    .map_buffer = &cgpu_map_buffer_metal,
    .unmap_buffer = &cgpu_unmap_buffer_metal,
    .free_buffer = &cgpu_free_buffer_metal,
    .create_buffer_view = &cgpu_create_buffer_view_metal,
    .free_buffer_view = &cgpu_free_buffer_view_metal,

    // Sampler APIs
    .create_sampler = &cgpu_create_sampler_metal,
    .free_sampler = &cgpu_free_sampler_metal,

    // Texture APIs
    .create_texture = &cgpu_create_texture_metal,
    .free_texture = &cgpu_free_texture_metal,
    .create_texture_view = &cgpu_create_texture_view_metal,
    .free_texture_view = &cgpu_free_texture_view_metal,

    // Swapchain APIs
    .create_swapchain = &cgpu_create_swapchain_metal,
    .acquire_next_image = &cgpu_acquire_next_image_metal,
    .free_swapchain = &cgpu_free_swapchain_metal,

    // CMDs
    .cmd_begin = &cgpu_cmd_begin_metal,
    .cmd_transfer_buffer_to_buffer = &cgpu_cmd_transfer_buffer_to_buffer_metal,
    .cmd_transfer_buffer_to_texture = &cgpu_cmd_transfer_buffer_to_texture_metal,
    .cmd_transfer_texture_to_texture = &cgpu_cmd_transfer_texture_to_texture_metal,
    .cmd_fill_buffer = &cgpu_cmd_fill_buffer_metal,
    .cmd_fill_buffer_n = &cgpu_cmd_fill_buffer_n_metal,
    .cmd_resource_barrier = &cgpu_cmd_resource_barrier_metal,
    .cmd_begin_query = &cgpu_cmd_begin_query_metal,
    .cmd_end_query = &cgpu_cmd_end_query_metal,
    .cmd_reset_query_pool = &cgpu_cmd_reset_query_pool_metal,
    .cmd_resolve_query = &cgpu_cmd_resolve_query_metal,
    .cmd_begin_event = &cgpu_cmd_begin_event_metal,
    .cmd_end_event = &cgpu_cmd_end_event_metal,
    .cmd_end = &cgpu_cmd_end_metal,

    // Compute CMDs
    .cmd_begin_compute_pass = &cgpu_cmd_begin_compute_pass_metal,
    .compute_encoder_bind_descriptor_buffer = &cgpu_compute_encoder_bind_descriptor_buffer_metal,
    .compute_encoder_bind_descriptor_set = &cgpu_compute_encoder_bind_descriptor_set_metal,
    .compute_encoder_push_constants = &cgpu_compute_encoder_push_constants_metal,
    .compute_encoder_bind_pipeline = &cgpu_compute_encoder_bind_pipeline_metal,
    .compute_encoder_set_threadgroup_size = &cgpu_compute_encoder_set_threadgroup_size_metal,
    .compute_encoder_dispatch = &cgpu_compute_encoder_dispatch_metal,
    .cmd_end_compute_pass = &cgpu_cmd_end_compute_pass_metal,
};

const CGPUProcTable* CGPU_MetalProcTable()
{
    return &tbl_metal;
}

// Instance APIs
CGPUInstanceId cgpu_create_instance_metal(CGPUInstanceDescriptor const* descriptor)
{
    CGPUInstance_Metal* MI = (CGPUInstance_Metal*)cgpu_calloc(1, sizeof(CGPUInstance_Metal));
    @autoreleasepool
    {
        NSArray<id<MTLDevice>>* mtlDevices = MetalUtil_GetAvailableMTLDeviceArray();
        MI->adapters = cgpu_calloc(mtlDevices.count, sizeof(CGPUAdapter_Metal));
        MI->adapters_count = mtlDevices.count;
        for (uint32_t i = 0; i < MI->adapters_count; i++)
        {
            MI->adapters[i].device.pDevice = mtlDevices[i];
        }
    }
    for (uint32_t i = 0; i < MI->adapters_count; i++)
    {
        // Query Adapter Informations
        MetalUtil_EnumFormatSupports(&MI->adapters[i]);
        MetalUtil_RecordAdapterDetail(&MI->adapters[i]);
    }
    return &MI->super;
}

void cgpu_query_instance_features_metal(CGPUInstanceId instance, struct CGPUInstanceFeatures* features)
{
    features->specialization_constant = true;
}

void cgpu_free_instance_metal(CGPUInstanceId instance)
{
    CGPUInstance_Metal* MI = (CGPUInstance_Metal*)instance;
    for (uint32_t i = 0; i < MI->adapters_count; i++)
    {
        MI->adapters[i].device.pDevice = nil;
    }
    cgpu_free(MI->adapters);
    cgpu_free(MI);
}

// Adapter APIs
void cgpu_enum_adapters_metal(CGPUInstanceId instance, CGPUAdapterId* const adapters, uint32_t* adapters_num)
{
    CGPUInstance_Metal* MI = (CGPUInstance_Metal*)instance;

    *adapters_num = MI->adapters_count;
    if (adapters != CGPU_NULLPTR)
    {
        for (uint32_t i = 0; i < MI->adapters_count; i++)
        {
            adapters[i] = &MI->adapters[i].super;
        }
    }
}

const CGPUAdapterDetail* cgpu_query_adapter_detail_metal(const CGPUAdapterId adapter)
{
    CGPUAdapter_Metal* MA = (CGPUAdapter_Metal*)adapter;
    return &MA->adapter_detail;
}

uint32_t cgpu_query_queue_count_metal(const CGPUAdapterId adapter, const ECGPUQueueType type)
{
    return UINT32_MAX;
}

// Device APIs
CGPUDeviceId cgpu_create_device_metal(CGPUAdapterId adapter, const CGPUDeviceDescriptor* desc)
{
    CGPUAdapter_Metal* MA = (CGPUAdapter_Metal*)adapter;
    // Create Requested Queues
    for (uint32_t i = 0; i < desc->queue_group_count; i++)
    {
        const CGPUQueueGroupDescriptor* queueGroup = desc->queue_groups + i;
        const ECGPUQueueType type = queueGroup->queue_type;
        MA->device.ppMtlQueues[type] = (__strong id<MTLCommandQueue>*)cgpu_calloc(queueGroup->queue_count, sizeof(id<MTLCommandQueue>));
        MA->device.pMtlQueueCounts[type] = queueGroup->queue_count;
        for (uint32_t j = 0u; j < queueGroup->queue_count; j++)
        {
            MA->device.ppMtlQueues[type][j] = [MA->device.pDevice newCommandQueueWithMaxCommandBufferCount:512];
        }
    }
    return &MA->device.super;
}

void cgpu_query_video_memory_info_metal(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes)
{
    CGPUDevice_Metal* D = (CGPUDevice_Metal*)device;
    *total = D->pDevice.recommendedMaxWorkingSetSize;
    *used_bytes = D->pDevice.currentAllocatedSize;
}

void cgpu_query_shared_memory_info_metal(const CGPUDeviceId device, uint64_t* total, uint64_t* used_bytes)
{
    CGPUDevice_Metal* D = (CGPUDevice_Metal*)device;
    if (D->pDevice.hasUnifiedMemory)
    {
        cgpu_query_video_memory_info_metal(device, total, used_bytes);
    }
    else
    {
        *total = UINT64_MAX;
        *used_bytes = 0;
    }
}

void cgpu_free_device_metal(CGPUDeviceId device)
{
    CGPUDevice_Metal* D = (CGPUDevice_Metal*)device;
    for (uint32_t i = 0; i < CGPU_QUEUE_TYPE_COUNT; i++)
    {
        if (D->ppMtlQueues[i] != NULL && D->pMtlQueueCounts[i] != 0)
        {
            for (uint32_t j = 0; j < D->pMtlQueueCounts[i]; j++)
            {
                D->ppMtlQueues[i][j] = nil;
            }
            cgpu_free(D->ppMtlQueues[i]);
        }
    }
    return;
}

// API Objects APIs
CGPUFenceId cgpu_create_fence_metal(CGPUDeviceId device)
{
    CGPUDevice_Metal* D = (CGPUDevice_Metal*)device;
    CGPUFence_Metal* F = (CGPUFence_Metal*)cgpu_calloc(1, sizeof(CGPUFence_Metal));
    F->pMTLEvent = [D->pDevice newSharedEvent];
    F->mFenceValue = 1;  // Start at 1 like D3D12
    return &F->super;
}

ECGPUFenceStatus cgpu_query_fence_status_metal(CGPUFenceId fence)
{
    CGPUFence_Metal* F = (CGPUFence_Metal*)fence;
    const uint64_t completedValue = F->pMTLEvent.signaledValue;
    if (completedValue < F->mFenceValue - 1)
        return CGPU_FENCE_STATUS_INCOMPLETE;
    else
        return CGPU_FENCE_STATUS_COMPLETE;
}

void cgpu_free_fence_metal(CGPUFenceId fence)
{
    CGPUFence_Metal* F = (CGPUFence_Metal*)fence;
    F->pMTLEvent = nil;
    cgpu_free(F);
}

void cgpu_wait_fences_metal(const CGPUFenceId* fences, uint32_t fence_count)
{
    for (uint32_t i = 0; i < fence_count; ++i)
    {
        CGPUFence_Metal* F = (CGPUFence_Metal*)fences[i];
        ECGPUFenceStatus fenceStatus = cgpu_query_fence_status(fences[i]);
        uint64_t fenceValue = F->mFenceValue - 1;
        if (fenceStatus == CGPU_FENCE_STATUS_INCOMPLETE)
        {
            // Use built-in wait functionality - wait indefinitely (UINT64_MAX timeout)
            [F->pMTLEvent waitUntilSignaledValue:fenceValue timeoutMS:UINT64_MAX];
        }
    }
}

CGPUSemaphoreId cgpu_create_semaphore_metal(CGPUDeviceId device)
{
    // Like D3D12, semaphores are just fences
    return (CGPUSemaphoreId)cgpu_create_fence_metal(device);
}

void cgpu_free_semaphore_metal(CGPUSemaphoreId semaphore)
{
    cgpu_free_fence_metal((CGPUFenceId)semaphore);
}

CGPURootSignaturePoolId cgpu_create_root_signature_pool_metal(CGPUDeviceId device, const struct CGPURootSignaturePoolDescriptor* desc)
{
    return CGPU_NULLPTR;
}

void cgpu_free_root_signature_pool_metal(CGPURootSignaturePoolId pool)
{
    
}

CGPURootSignatureId cgpu_create_root_signature_metal(CGPUDeviceId device, const struct CGPURootSignatureDescriptor* desc)
{
    CGPURootSignature_Metal* RS = (CGPURootSignature_Metal*)cgpu_calloc(1, sizeof(CGPURootSignature_Metal));
    @autoreleasepool {
        for (uint32_t i = 0; i < desc->shader_count; i++)
        {
            const struct CGPUShaderEntryDescriptor* shader_entry = desc->shaders + i;
            const struct CGPUShaderLibrary_Metal* lib = (const struct CGPUShaderLibrary_Metal*)shader_entry->library;
            // TODO: CONSTANT SPECIALIZATION
            NSString* entryName = [NSString stringWithUTF8String:shader_entry->entry];
            RS->mtlFunctions[i] = [lib->mtlLibrary newFunctionWithName: entryName];
        }
    }
    // CREATE DEFAULT PIPELINE
    NSError* error = nil;
    MTLAutoreleasedComputePipelineReflection reflection;
    RS->mtlPipelineState = [((CGPUDevice_Metal*)device)->pDevice 
        newComputePipelineStateWithFunction: RS->mtlFunctions[0]
        options: MTLPipelineOptionBindingInfo | MTLPipelineOptionBufferTypeInfo
        reflection:&reflection
        error:&error
    ];
    if (error != nil || RS->mtlPipelineState == nil)
    {
        cgpu_error("Failed to create MTLComputePipelineState: %s", error.localizedDescription.UTF8String);
        for (uint32_t i = 0; i < CGPU_SHADER_STAGE_COUNT; i++)
            RS->mtlFunctions[i] = nil;
        cgpu_free(RS);
        return CGPU_NULLPTR;
    }
    // REFLECTION
    RS->super.push_constants = cgpu_calloc(desc->push_constant_count, sizeof(CGPUShaderResource));
    NSMutableArray<id<MTLBinding>>* unique_bindings = [[NSMutableArray<id<MTLBinding>> alloc] init];
    for (uint32_t i = 0; i < reflection.bindings.count; i++)
    {
        id<MTLBinding> pending = reflection.bindings[i];
        for (id<MTLBinding> exist in unique_bindings)
        {
            if (exist.index == pending.index)
                pending = nil;
        }
        if (pending != nil)
            [unique_bindings addObject:pending];
    }
    RS->super.table_count = unique_bindings.count;
    RS->super.tables = cgpu_calloc(RS->super.table_count, sizeof(CGPUParameterTable));
    for (uint32_t i = 0; i < unique_bindings.count; i++)
    {
        CGPUParameterTable* table = RS->super.tables + i;
        table->metal.arg_buf_size = 0;
        if (unique_bindings[i].type == MTLBindingTypeBuffer)
        {
            id<MTLBufferBinding> SRT = (id<MTLBufferBinding>)unique_bindings[i];
            MTLStructType* SRTLayout = SRT.bufferStructType;
            table->metal.arg_buf_size = SRT.bufferDataSize;
            table->set_index = SRT.index;
            table->resources = cgpu_calloc(SRTLayout.members.count, sizeof(CGPUShaderResource));
            for (uint32_t j = 0; j < SRTLayout.members.count; j++)
            {
                CGPUShaderResource resource;
                MetalUtil_GetShaderResourceType(SRT, SRT.index, SRTLayout.members[j], &resource);
                if (resource.view_usages == CGPU_BUFFER_VIEW_USAGE_PUSH_CONSTANT)
                {
                    RS->super.push_constants[0] = resource;
                    RS->super.push_constant_count = 1;
                }
                else
                {
                    *(table->resources + table->resources_count) = resource;
                    table->resources_count += 1;
                }
            }
        }
        else
        {
            SKR_ASSERT(false && "Unsupported binding type in MTLBindingAccess");
        }
    }
    
    // 在反射阶段解析静态采样器的绑定信息
    RS->staticSamplerCount = desc->static_sampler_count;
    if (desc->static_sampler_count > 0) {
        RS->staticSamplers = (CGPUStaticSampler_Metal*)cgpu_calloc(desc->static_sampler_count, sizeof(CGPUStaticSampler_Metal));
        
        for (uint32_t i = 0; i < desc->static_sampler_count; i++) {
            CGPUSampler_Metal* sampler = (CGPUSampler_Metal*)desc->static_samplers[i];
            const char8_t* samplerName = desc->static_sampler_names[i];
            
            RS->staticSamplers[i].mtlSamplerState = sampler->mtlSamplerState;
            RS->staticSamplers[i].setIndex = UINT32_MAX;
            RS->staticSamplers[i].resourceOffset = UINT32_MAX;
            
            // 通过名称查找对应的shader resource，获取set index和offset
            skr_hash name_hash = skr_hash_of(samplerName, strlen((const char*)samplerName), SKR_DEFAULT_HASH_SEED);
            for (uint32_t j = 0; j < RS->super.table_count; j++) {
                CGPUParameterTable* table = RS->super.tables + j;
                for (uint32_t k = 0; k < table->resources_count; k++) {
                    CGPUShaderResource* resource = table->resources + k;
                    if (resource->name_hash == name_hash && 
                        strcmp((const char*)resource->name, (const char*)samplerName) == 0 &&
                        resource->type == CGPU_RESOURCE_TYPE2_SAMPLER) {
                        RS->staticSamplers[i].setIndex = table->set_index;
                        RS->staticSamplers[i].resourceOffset = resource->offset;
                        break;
                    }
                }
                if (RS->staticSamplers[i].setIndex != UINT32_MAX) break;
            }
        }
        
        // 为静态采样器创建Set和ArgBuffer映射
        NSMutableSet<NSNumber*>* uniqueSets = [NSMutableSet set];
        for (uint32_t i = 0; i < RS->staticSamplerCount; i++) {
            if (RS->staticSamplers[i].setIndex != UINT32_MAX) {
                [uniqueSets addObject:@(RS->staticSamplers[i].setIndex)];
            }
        }
        
        RS->staticSamplerSetCount = (uint32_t)uniqueSets.count;
        if (RS->staticSamplerSetCount > 0) {
            RS->staticSamplerSets = (CGPUStaticSamplerSet_Metal*)cgpu_calloc(RS->staticSamplerSetCount, sizeof(CGPUStaticSamplerSet_Metal));
            
            uint32_t setIndex = 0;
            for (NSNumber* setNum in uniqueSets) {
                uint32_t currentSetIndex = setNum.unsignedIntValue;
                RS->staticSamplerSets[setIndex].setIndex = currentSetIndex;
                
                // 计算该set的ArgBuffer大小
                uint32_t maxOffset = 0;
                for (uint32_t i = 0; i < RS->staticSamplerCount; i++) {
                    if (RS->staticSamplers[i].setIndex == currentSetIndex) {
                        uint32_t offset = RS->staticSamplers[i].resourceOffset + sizeof(uint64_t);
                        maxOffset = (maxOffset > offset) ? maxOffset : offset;
                    }
                }
                
                // 创建ArgBuffer
                CGPUDevice_Metal* D = (CGPUDevice_Metal*)device;
                RS->staticSamplerSets[setIndex].argBuffer = [D->pDevice newBufferWithLength:maxOffset 
                                                                                    options:MTLResourceStorageModeShared];
                RS->staticSamplerSets[setIndex].argBuffer.label = [NSString stringWithFormat:@"StaticSamplerArgBuffer_Set%u", currentSetIndex];
                setIndex++;
            }
        } else {
            RS->staticSamplerSets = nil;
        }
    } else {
        RS->staticSamplers = nil;
        RS->staticSamplerSets = nil;
        RS->staticSamplerSetCount = 0;
    }
    
    return &RS->super;
}

void cgpu_free_root_signature_metal(CGPURootSignatureId root_signature)
{
    CGPURootSignature_Metal* RS = (CGPURootSignature_Metal*)root_signature;

    for (uint32_t i = 0; i < RS->super.table_count; i++)
    {
        CGPUParameterTable* table = RS->super.tables + i;
        for (uint32_t j = 0; j < table->resources_count; j++)
        {
            CGPUShaderResource* resource = table->resources + j;
            cgpu_free((void*)resource->name);
        }
        cgpu_free(table->resources);
    }
    cgpu_free(RS->super.tables);

    for (uint32_t j = 0; j < RS->super.push_constant_count; j++)
    {
        CGPUShaderResource* resource = RS->super.push_constants + j;
        cgpu_free((void*)resource->name);
    }
    cgpu_free(RS->super.push_constants);
    
    // 释放静态采样器
    if (RS->staticSamplers) {
        for (uint32_t i = 0; i < RS->staticSamplerCount; i++) {
            RS->staticSamplers[i].mtlSamplerState = nil;
        }
        cgpu_free(RS->staticSamplers);
        RS->staticSamplers = nil;
    }
    
    // 释放静态采样器Set和ArgBuffer
    if (RS->staticSamplerSets) {
        for (uint32_t i = 0; i < RS->staticSamplerSetCount; i++) {
            RS->staticSamplerSets[i].argBuffer = nil;
        }
        cgpu_free(RS->staticSamplerSets);
        RS->staticSamplerSets = nil;
    }

    for (uint32_t i = 0; i < CGPU_SHADER_STAGE_COUNT; i++)
    {
        if (RS->mtlFunctions[i])
            RS->mtlFunctions[i] = nil;
    }
    RS->mtlPipelineState = nil;
    cgpu_free(RS);
}

CGPUQueryPoolId cgpu_create_query_pool_metal(CGPUDeviceId device, const struct CGPUQueryPoolDescriptor* desc)
{
    SKR_ASSERT(desc->type == CGPU_QUERY_TYPE_TIMESTAMP);
    CGPUQueryPool_Metal* P = (CGPUQueryPool_Metal*)cgpu_calloc(1, sizeof(CGPUQueryPool_Metal));
    P->queryType = desc->type;
    P->super.count = desc->query_count;
    return &P->super;
}

void cgpu_free_query_pool_metal(CGPUQueryPoolId pool)
{
    CGPUQueryPool_Metal* P = (CGPUQueryPool_Metal*)pool;
    P->mtlCounterSampleBuffer = nil;
    cgpu_free(P);
}

// fill argument
typedef struct BUFFER { uint64_t ptr; uint64_t size; } BUFFER;
typedef struct TBUFFER { MTLResourceID id; uint64_t size; } TBUFFER;
typedef struct CBUFFER { uint64_t ptr; } CBUFFER;
typedef struct TEXTURE { MTLResourceID id; } TEXTURE;
typedef struct SAMPLER { MTLResourceID id; } SAMPLER;
typedef struct TLAS { MTLResourceID id; } TLAS;
static const uint64_t kMaxRegisterSize = sizeof(uint64_t) * 2;

CGPUDescriptorBufferId cgpu_create_descriptor_buffer_metal(CGPUDeviceId device, const struct CGPUDescriptorBufferDescriptor* desc)
{
    CGPUDescriptorBuffer_Metal* DB = (CGPUDescriptorBuffer_Metal*)cgpu_calloc(1, sizeof(CGPUDescriptorBuffer_Metal));
    if (desc->count > 0)
    {
        DB->mtlArgsCache = (__unsafe_unretained id*)cgpu_calloc(desc->count, sizeof(id));
        DB->mtlTexelBufferCache = (__strong id*)cgpu_calloc(desc->count, sizeof(id));
        
        CGPUBufferDescriptor buffer_desc = {};
        buffer_desc.size = desc->count * kMaxRegisterSize;
        buffer_desc.name = "ArgumentBuffer(DescriptorBuffer)";
        buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
        DB->mtlArgumentBuffer = cgpu_create_buffer(device, &buffer_desc);
    }
    DB->super.size = desc->count;
    return &DB->super;
}

void cgpu_update_descriptor_buffer_metal(CGPUDescriptorBufferId buffer_id, const struct CGPUDescriptorBufferElement* elements, uint32_t count)
{
    CGPUDescriptorBuffer_Metal* buffer = (CGPUDescriptorBuffer_Metal*)buffer_id;
    CGPUDevice_Metal* D = (CGPUDevice_Metal*)buffer->super.device;
    CGPUBuffer_Metal* ArgumentBuffer = (CGPUBuffer_Metal*)buffer->mtlArgumentBuffer;
    uint8_t* pArg = ArgumentBuffer->mtlBuffer.contents + ArgumentBuffer->mOffset;
    
    for (uint32_t i = 0; i < count; i++)
    {
        const CGPUDescriptorBufferElement* element = &elements[i];
        if (element->resource_type == CGPU_RESOURCE_TYPE2_TEXTURE)
        {
            TEXTURE* pTextureArg = ((TEXTURE*)pArg) + element->index;
            id<MTLTexture> T = MetalUtil_CreateTextureView(&D->super, &element->texture);
            struct TEXTURE ARG = { T.gpuResourceID };
            memcpy(pTextureArg, &ARG, sizeof(ARG));
            buffer->mtlArgsCache[element->index] = T;
        }
        else if (element->resource_type == CGPU_RESOURCE_TYPE2_BUFFER)
        {
            BUFFER* pBufferArg = ((BUFFER*)pArg) + element->index;
            const CGPUBuffer_Metal* B = (const CGPUBuffer_Metal*)element->buffer.buffer;
            
            // 检查是否是 texel buffer
            if (element->buffer.view_usages & CGPU_BUFFER_VIEW_USAGE_UAV_TEXEL ||
                element->buffer.view_usages & CGPU_BUFFER_VIEW_USAGE_SRV_TEXEL)
            {
                // 取消持有旧的 texel buffer（如果存在）
                if (element->index < buffer->super.size && buffer->mtlTexelBufferCache[element->index] != nil)
                {
                    buffer->mtlTexelBufferCache[element->index] = nil;
                }
                
                const uint32_t BufferViewSize = element->buffer.size ? element->buffer.size : (B->super.info->size - element->buffer.offset);
                bool writable = (element->buffer.view_usages & CGPU_BUFFER_VIEW_USAGE_UAV_TEXEL) != 0;
                const uint32_t bytesPerPixel = FormatUtil_BitSizeOfBlock(element->buffer.texel.format) / 8;
                const uint32_t texelCount = BufferViewSize / bytesPerPixel;
                MTLTextureDescriptor* texelDesc = [[MTLTextureDescriptor alloc] init];
                texelDesc.textureType = MTLTextureTypeTextureBuffer;
                texelDesc.pixelFormat = MetalUtil_TranslatePixelFormat(element->buffer.texel.format);
                texelDesc.width = texelCount;
                texelDesc.height = 1;
                texelDesc.depth = 1;
                texelDesc.usage = writable ? MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite: MTLTextureUsageShaderRead;
                texelDesc.resourceOptions = B->mtlResourceOptions;
                const uint32_t bytesPerRow = texelCount * bytesPerPixel;
                id<MTLTexture> texelBuffer = [B->mtlBuffer newTextureWithDescriptor:texelDesc 
                                                                offset:B->mOffset + element->buffer.offset 
                                                                bytesPerRow:bytesPerRow];
                
                if (texelBuffer == nil) {
                    cgpu_error("Failed to create texel buffer: format=%d, width=%u, bytesPerPixel=%u, bytesPerRow=%u, offset=%llu", 
                               element->buffer.texel.format, texelCount, bytesPerPixel, bytesPerRow, B->mOffset + element->buffer.offset);
                }
                
                [texelDesc release];
                
                // 持有新的 texel buffer 引用
                buffer->mtlTexelBufferCache[element->index] = texelBuffer;
                
                // 创建 TBUFFER 结构用于 shader 绑定
                struct TBUFFER ARG = { 
                    texelBuffer.gpuResourceID,
                    BufferViewSize
                };
                memcpy(pBufferArg, &ARG, sizeof(ARG));
                buffer->mtlArgsCache[element->index] = texelBuffer;
            }
            else
            {
                // 普通 buffer 绑定
                struct BUFFER ARG = { 
                    B->mtlBuffer.gpuAddress + B->mOffset + element->buffer.offset,
                    B->super.info->size
                };
                memcpy(pBufferArg, &ARG, sizeof(ARG));
                buffer->mtlArgsCache[element->index] = B->mtlBuffer;
            }
        }
    }
}

void cgpu_copy_descriptor_buffer_metal(CGPUDescriptorBufferId src_id, CGPUDescriptorBufferId dest_id, CGPUBufferRange src_range, CGPUBufferRange dst_range)
{
    CGPUDescriptorBuffer_Metal* src = (CGPUDescriptorBuffer_Metal*)src_id;
    CGPUDescriptorBuffer_Metal* dest = (CGPUDescriptorBuffer_Metal*)dest_id;
    CGPUDevice_Metal* D = (CGPUDevice_Metal*)src->super.device;

    cgpu_assert(src_range.size == dst_range.size && "Copy ranges must be same size");

    uint32_t count = (uint32_t)src_range.size;
    uint32_t src_offset = (uint32_t)src_range.offset;
    uint32_t dst_offset = (uint32_t)dst_range.offset;
    
    uint8_t* pSrc = ((CGPUBuffer_Metal*)src->mtlArgumentBuffer)->mtlBuffer.contents + ((CGPUBuffer_Metal*)src->mtlArgumentBuffer)->mOffset + src_offset;
    uint8_t* pDst = ((CGPUBuffer_Metal*)dest->mtlArgumentBuffer)->mtlBuffer.contents + ((CGPUBuffer_Metal*)dest->mtlArgumentBuffer)->mOffset + dst_offset;
    memcpy(pDst, pSrc, count * kMaxRegisterSize);
}

void cgpu_free_descriptor_buffer_metal(CGPUDescriptorBufferId set)
{
    CGPUDescriptorBuffer_Metal* DB = (CGPUDescriptorBuffer_Metal*)set;
    // DS->mtlArgumentEncoder = nil;
    if (DB->mtlArgumentBuffer)
    {
        cgpu_free_buffer(DB->mtlArgumentBuffer);
    }
    
    // 清理 texel buffer 引用
    if (DB->mtlTexelBufferCache)
    {
        for (uint32_t i = 0; i < DB->super.size; i++)
        {
            if (DB->mtlTexelBufferCache[i] != nil)
            {
                DB->mtlTexelBufferCache[i] = nil;
            }
        }
    }
    
    cgpu_free(DB->mtlArgsCache);
    cgpu_free(DB->mtlTexelBufferCache);
    cgpu_free(DB);
}

CGPUDescriptorSetId cgpu_create_descriptor_set_metal(CGPUDeviceId device, const struct CGPUDescriptorSetDescriptor* desc)
{
    CGPUDescriptorSet_Metal* DS = (CGPUDescriptorSet_Metal*)cgpu_calloc(1, sizeof(CGPUDescriptorSet_Metal));
    CGPURootSignature_Metal* RS = (CGPURootSignature_Metal*)desc->root_signature;
    CGPUParameterTable* set_table = RS->super.tables + desc->set_index;
    DS->mtlBindSlots = cgpu_calloc(set_table->resources_count, sizeof(BindSlot_Metal));
    DS->mtlReadArgsCache = (__unsafe_unretained id*)cgpu_calloc(set_table->resources_count, sizeof(id));
    DS->mtlReadWriteArgsCache = (__unsafe_unretained id*)cgpu_calloc(set_table->resources_count, sizeof(id));
    if (set_table->resources_count > 0)
    {
        CGPUBufferDescriptor buffer_desc = {};
        buffer_desc.size = set_table->metal.arg_buf_size;
        buffer_desc.name = "ArgumentBuffer(DescriptorSet)";
        buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
        DS->mtlArgumentBuffer = cgpu_create_buffer(device, &buffer_desc);
    }
    return &DS->super;
}

void cgpu_update_descriptor_set_metal(CGPUDescriptorSetId set, const struct CGPUDescriptorData* datas, uint32_t count)
{
    CGPUDescriptorSet_Metal* DS = (CGPUDescriptorSet_Metal*)set;
    CGPURootSignature_Metal* RS = (CGPURootSignature_Metal*)DS->super.root_signature;
    CGPUParameterTable* set_table = RS->super.tables + DS->super.index;
    if (DS->mtlArgumentBuffer)
    {
        CGPUBuffer_Metal* ArgumentBuffer = (CGPUBuffer_Metal*)DS->mtlArgumentBuffer;
        uint8_t* pBuf = ArgumentBuffer->mtlBuffer.contents + ArgumentBuffer->mOffset;
        for (uint32_t i = 0; i < count; i++)
        {
            const CGPUDescriptorData* data = datas + i;

            // find correct binding slot
            uint32_t binding_index = data->by_index.binding;
            CGPUShaderResource* resource = CGPU_NULLPTR;
            if (data->by_name.name != CGPU_NULLPTR)
            {
                skr_hash name_hash = skr_hash_of(data->by_name.name, strlen((const char*)data->by_name.name), SKR_DEFAULT_HASH_SEED);
                for (uint32_t j = 0; j < set_table->resources_count; j++)
                {
                    CGPUShaderResource* temp_resource = set_table->resources + j;
                    if (temp_resource->name_hash == name_hash && 
                        strcmp((const char*)data->by_name.name, (const char*)temp_resource->name) == 0)
                    {
                        binding_index = temp_resource->binding;
                        resource = temp_resource;
                        break;
                    }
                }
            }
            else
            {
                for (uint32_t j = 0; j < set_table->resources_count; j++)
                {
                    CGPUShaderResource* temp_resource = set_table->resources + j;
                    if (temp_resource->binding == binding_index)
                    {
                        resource = temp_resource;
                        break;
                    }
                }
            }

            uint8_t* pArg = pBuf + resource->offset;
            switch (resource->type)
            {
                case CGPU_RESOURCE_TYPE2_SAMPLER:
                {
                    if (resource->size != UINT32_MAX)
                    {
                        const bool writable = (resource->view_usages == CGPU_TEXTURE_VIEW_USAGE_UAV);
                        for (uint32_t j = 0; j < data->count; j++)
                        {
                            SAMPLER* pSamplerArg = ((SAMPLER*)pArg) + j;
                            const CGPUSampler_Metal* S = (const CGPUSampler_Metal*)data->samplers[j];
                            struct SAMPLER ARG = { S->mtlSamplerState.gpuResourceID };
                            memcpy(pSamplerArg, &ARG, sizeof(ARG));
                        }
                    }
                    // bindless case, in this case bindpoint is a device/constant pointer
                    else 
                    {
                        SKR_UNIMPLEMENTED_FUNCTION();
                    }
                }
                break;
                case CGPU_RESOURCE_TYPE2_BUFFER:
                {
                    if (resource->view_usages != CGPU_BUFFER_VIEW_USAGE_CBV)
                    {
                        const MTLResourceUsage usage = (resource->view_usages == CGPU_TEXTURE_VIEW_USAGE_UAV) ? MTLResourceUsageRead | MTLResourceUsageWrite : MTLResourceUsageRead;
                        if (resource->size != UINT32_MAX)
                        {
                            for (uint32_t j = 0; j < data->count; j++)
                            {
                                BUFFER* pBufferArg = ((BUFFER*)pArg) + j;
                                const CGPUBufferView_Metal* B = (const CGPUBufferView_Metal*)data->buffers[j];
                                struct BUFFER ARG = { 
                                    B->mtlBuffer.gpuAddress + B->mOffset,
                                    B->super.info->size
                                };
                                MetalUtil_DSBindResourceAtIndex(DS, binding_index, B->mtlBuffer, usage);
                                memcpy(pBufferArg, &ARG, sizeof(ARG));
                            }
                        }
                        // bindless case, in this case bindpoint is a device/constant pointer
                        else 
                        {
                            SKR_UNIMPLEMENTED_FUNCTION();
                        }
                    }
                    else if (resource->view_usages == CGPU_BUFFER_VIEW_USAGE_SRV_TEXEL || 
                             resource->view_usages == CGPU_BUFFER_VIEW_USAGE_UAV_TEXEL)
                    {
                        const MTLResourceUsage usage = (resource->view_usages == CGPU_TEXTURE_VIEW_USAGE_UAV) ? MTLResourceUsageRead | MTLResourceUsageWrite : MTLResourceUsageRead;
                        if (resource->size != UINT32_MAX)
                        {
                            for (uint32_t j = 0; j < data->count; j++)
                            {
                                BUFFER* pBufferArg = ((BUFFER*)pArg) + j;
                                const CGPUBufferView_Metal* B = (const CGPUBufferView_Metal*)data->buffers[j];
                                struct TBUFFER ARG = { 
                                    B->mtlTextureBuffer.gpuResourceID,
                                    B->super.info->size
                                };
                                MetalUtil_DSBindResourceAtIndex(DS, binding_index, B->mtlTextureBuffer, usage);
                                memcpy(pBufferArg, &ARG, sizeof(ARG));
                            }
                        }
                        // bindless case, in this case bindpoint is a device/constant pointer
                        else 
                        {
                            SKR_UNIMPLEMENTED_FUNCTION();
                        }
                    }
                    else
                    {
                        SKR_ASSERT(resource->size == 1 && "Uniform buffer must have size 1");
                        CBUFFER* pBufferArg = (CBUFFER*)pArg;
                        const CGPUBuffer_Metal* B = (const CGPUBuffer_Metal*)data->buffers[0];
                        struct CBUFFER ARG = { 
                            B->mtlBuffer.gpuAddress + B->mOffset
                        };
                        MetalUtil_DSBindResourceAtIndex(DS, binding_index, B->mtlBuffer, MTLResourceUsageRead);
                        memcpy(pBufferArg, &ARG, sizeof(ARG));
                    }
                }
                break;
                case CGPU_RESOURCE_TYPE2_TEXTURE:
                {
                    if (resource->size != UINT32_MAX)
                    {
                        const bool writable = (resource->view_usages == CGPU_TEXTURE_VIEW_USAGE_UAV);
                        for (uint32_t j = 0; j < data->count; j++)
                        {
                            TEXTURE* pTextureArg = ((TEXTURE*)pArg) + j;
                            const CGPUTextureView_Metal* T = (const CGPUTextureView_Metal*)data->textures[j];
                            struct TEXTURE ARG = { T->pTextureView.gpuResourceID };
                            MetalUtil_DSBindResourceAtIndex(DS, binding_index, T->pTextureView, 
                                writable ? MTLResourceUsageRead | MTLResourceUsageWrite : MTLResourceUsageRead);
                            memcpy(pTextureArg, &ARG, sizeof(ARG));
                        }
                    }
                    // bindless case, in this case bindpoint is a device/constant pointer
                    else 
                    {
                        SKR_UNIMPLEMENTED_FUNCTION();
                    }
                }
                break;
                case CGPU_RESOURCE_TYPE2_ACCELERATION_STRUCTURE:
                {
                    SKR_ASSERT(resource->size == 1 && "TLAS must have size 1");
                    TLAS* pTLAS = (TLAS*)pArg;
                    const CGPUAccelerationStructure_Metal* AS = (const CGPUAccelerationStructure_Metal*)data->acceleration_structures[0];
                    struct TLAS ARG = { 
                        AS->mtlAS.gpuResourceID
                    };
                    MetalUtil_DSBindResourceAtIndex(DS, binding_index, AS->mtlAS, MTLResourceUsageRead);
                    DS->pBoundAS = AS;
                    memcpy(pTLAS, &ARG, sizeof(ARG));
                }
                break;
                default:
                {
                    SKR_ASSERT(false && "Unsupported binding type in MTLArgumentEncoder");
                }
                break;
            }
        }
    }
}

void cgpu_free_descriptor_set_metal(CGPUDescriptorSetId set)
{
    CGPUDescriptorSet_Metal* DS = (CGPUDescriptorSet_Metal*)set;
    // DS->mtlArgumentEncoder = nil;
    if (DS->mtlArgumentBuffer)
    {
        cgpu_free_buffer(DS->mtlArgumentBuffer);
    }
    cgpu_free(DS->mtlBindSlots);
    cgpu_free(DS->mtlReadArgsCache);
    cgpu_free(DS->mtlReadWriteArgsCache);
    cgpu_free(DS);
}

CGPUComputePipelineId cgpu_create_compute_pipeline_metal(CGPUDeviceId device, const struct CGPUComputePipelineDescriptor *desc)
{
    CGPUComputePipeline_Metal* CP = (CGPUComputePipeline_Metal*)cgpu_calloc(1, sizeof(CGPUComputePipeline_Metal));

    CGPURootSignature_Metal* RS = (CGPURootSignature_Metal*)desc->root_signature;
    CP->mtlPipelineState = RS->mtlPipelineState;
    return &CP->super;
}

void cgpu_free_compute_pipeline_metal(CGPUComputePipelineId pipeline)
{
    CGPUComputePipeline_Metal* CP = (CGPUComputePipeline_Metal*)pipeline;
    CP->mtlPipelineState = nil;
    cgpu_free(CP);
}

// Queue APIs
CGPUQueueId cgpu_get_queue_metal(CGPUDeviceId device, ECGPUQueueType type, uint32_t index)
{
    CGPUQueue_Metal* MQ = (CGPUQueue_Metal*)cgpu_calloc(1, sizeof(CGPUQueue_Metal));
    CGPUDevice_Metal* MD = (CGPUDevice_Metal*)device;
    MQ->super.device = device;  // Set device reference
    MQ->super.type = type;      // Set queue type
    MQ->mtlCommandQueue = MD->ppMtlQueues[type][index];
    // Don't create fence here - it's causing issues
    MQ->pFence = NULL;
    return &MQ->super;
}

void cgpu_submit_queue_metal(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc)
{
    uint32_t CmdCount = desc->cmds_count;
    CGPUCommandBuffer_Metal** Cmds = (CGPUCommandBuffer_Metal**)desc->cmds;
    CGPUQueue_Metal* Q = (CGPUQueue_Metal*)queue;
    CGPUFence_Metal* F = (CGPUFence_Metal*)desc->signal_fence;
    
    // Assert that given cmd list and given params are valid
    cgpu_assert(CmdCount > 0);
    cgpu_assert(Cmds);
    cgpu_assert(Q->mtlCommandQueue);

    @autoreleasepool
    {
        // Flush all command encoders
        for (uint32_t i = 0; i < CmdCount; ++i)
        {
            MetalUtil_FlushUtilEncoders(Cmds[i], MTLUtilEncoderTypeAS | MTLUtilEncoderTypeBlit);
        }

        // Wait semaphores - encode wait on first command buffer
        if (desc->wait_semaphore_count > 0)
        {
            CGPUFence_Metal** WaitSemaphores = (CGPUFence_Metal**)desc->wait_semaphores;
            for (uint32_t i = 0; i < desc->wait_semaphore_count; ++i)
            {
                [Cmds[0]->mtlCommandBuffer encodeWaitForEvent:WaitSemaphores[i]->pMTLEvent 
                                                         value:WaitSemaphores[i]->mFenceValue - 1];
            }
        }

        // Signal fence on last command buffer
        if (F)
        {
            [Cmds[CmdCount - 1]->mtlCommandBuffer encodeSignalEvent:F->pMTLEvent 
                                                               value:F->mFenceValue++];
        }

        // Signal Semaphores on last command buffer
        if (desc->signal_semaphore_count > 0)
        {
            CGPUFence_Metal** SignalSemaphores = (CGPUFence_Metal**)desc->signal_semaphores;
            for (uint32_t i = 0; i < desc->signal_semaphore_count; i++)
            {
                [Cmds[CmdCount - 1]->mtlCommandBuffer encodeSignalEvent:SignalSemaphores[i]->pMTLEvent 
                                                                   value:SignalSemaphores[i]->mFenceValue++];
            }
        }
        
        // Add error handlers (optional, but useful for debugging)
        for (uint32_t i = 0; i < CmdCount; ++i)
        {
            [Cmds[i]->mtlCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
                if (buffer.error != nil)
                    cgpu_error("Failed to execute commands with error (%s)", [buffer.error.description UTF8String]);
            }];
        }

        // Execute - commit all command buffers
        for (uint32_t i = 0; i < CmdCount; ++i)
        {
            [Cmds[i]->mtlCommandBuffer commit];
        }
    }
}

void cgpu_wait_queue_idle_metal(CGPUQueueId queue)
{
    CGPUQueue_Metal* Q = (CGPUQueue_Metal*)queue;
    
    // Simple approach: create a command buffer and wait for it to complete
    id<MTLCommandBuffer> waitCmd = [Q->mtlCommandQueue commandBufferWithUnretainedReferences];
    [waitCmd commit];
    [waitCmd waitUntilCompleted];
}

void cgpu_free_queue_metal(CGPUQueueId queue)
{
    CGPUQueue_Metal* Q = (CGPUQueue_Metal*)queue;
    Q->mtlCommandQueue = nil;
    if (Q->pFence)
    {
        cgpu_free_fence_metal(&Q->pFence->super);
    }
    cgpu_free(Q);
}

// Command APIs
CGPUCommandPoolId cgpu_create_command_pool_metal(CGPUQueueId queue, const CGPUCommandPoolDescriptor* desc)
{
    CGPUCommandPool_Metal* PQ = (CGPUCommandPool_Metal*)cgpu_calloc(1, sizeof(CGPUCommandPool_Metal));
    return &PQ->super;
}

void cgpu_reset_command_pool_metal(CGPUCommandPoolId pool)
{
    (void)pool;
}

CGPUCommandBufferId cgpu_create_command_buffer_metal(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc)
{
    CGPUCommandBuffer_Metal* MB = (CGPUCommandBuffer_Metal*)cgpu_calloc(1, sizeof(CGPUCommandBuffer_Metal));
    return &MB->super;
}

void cgpu_free_command_buffer_metal(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_Metal* MB = (CGPUCommandBuffer_Metal*)cmd;
    MB->mtlCommandBuffer = nil;
    MB->cmptEncoder.mtlComputeEncoder = nil;
    MB->renderEncoder.mtlRenderEncoder = nil;
    cgpu_free(MB);
}

void cgpu_free_command_pool_metal(CGPUCommandPoolId pool)
{
    CGPUCommandPool_Metal* PQ = (CGPUCommandPool_Metal*)pool;
    cgpu_free(PQ);
}

// Shader APIs
CGPUShaderLibraryId cgpu_create_shader_library_metal(CGPUDeviceId device, const CGPUShaderLibraryDescriptor* desc)
{
    CGPUShaderLibrary_Metal* ML = (CGPUShaderLibrary_Metal*)cgpu_calloc(1, sizeof(CGPUShaderLibrary_Metal));
    dispatch_data_t byteCode = dispatch_data_create(desc->code, desc->code_size, nil, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
    NSError* error = nil;
    ML->mtlLibrary = [((CGPUDevice_Metal*)device)->pDevice newLibraryWithData: byteCode error:&error];
    if (error)
    {
        cgpu_free(ML);
        return NULL;
    }
    return &ML->super;
}

void cgpu_free_shader_library_metal(CGPUShaderLibraryId library)
{
    CGPUShaderLibrary_Metal* ML = (CGPUShaderLibrary_Metal*)library;
    ML->mtlLibrary = nil;
    cgpu_free(ML);
}

// Buffer APIs
CGPUBufferId cgpu_create_buffer_metal(CGPUDeviceId device, const CGPUBufferDescriptor* desc)
{
    CGPUDevice_Metal* D = (CGPUDevice_Metal*)device;
    CGPUAdapter_Metal* A = (CGPUAdapter_Metal*)D->super.adapter;
    CGPUBuffer_Metal* B = (CGPUBuffer_Metal*)cgpu_calloc(1, sizeof(CGPUBuffer_Metal) + sizeof(CGPUBufferInfo));
    CGPUBufferInfo* pInfo = (CGPUBufferInfo*)(B + 1);
    pInfo->size = desc->size;
    // TODO: UMA OPTIMIZATION
    // const bool isUMA = A->adapter_detail.is_uma;

    if (desc->usages & CGPU_BUFFER_USAGE_CONSTANT_BUFFER)
        pInfo->size = CGPU_ALIGN(pInfo->size, A->adapter_detail.uniform_buffer_alignment);
    
    MemoryType memoryType = MetalUtil_MemoryUsageToMemoryType(desc->memory_usage);
    MTLResourceOptions options = MetalUtil_MemoryTypeToResourceOptions(memoryType);
    
    // TODO: USE MTL-VMA TO OPTIMIZE
    B->mtlBuffer = [D->pDevice newBufferWithLength:pInfo->size options:options];
    B->mOffset = 0;
    B->mtlResourceOptions = options;
    
    if (desc->name) {
        NSString* bufferName = [NSString stringWithUTF8String:(const char*)desc->name];
        B->mtlBuffer.label = bufferName;
    }

    if (desc->flags & CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT)
        pInfo->cpu_mapped_address = B->mtlBuffer.contents + B->mOffset;

    pInfo->usages = desc->usages;
    pInfo->memory_usage = desc->memory_usage;
    B->super.info = pInfo;
    return &B->super;
}

void cgpu_map_buffer_metal(CGPUBufferId buffer, const struct CGPUBufferRange* desc)
{
    CGPUBuffer_Metal* B = (CGPUBuffer_Metal*)buffer;
    if (desc->offset + desc->size > B->super.info->size)
    {
        SKR_ASSERT(false && "Buffer range out of bounds");
        return;
    }
    CGPUBufferInfo* pInfo = (CGPUBufferInfo*)B->super.info;
    pInfo->cpu_mapped_address = B->mtlBuffer.contents + B->mOffset;
}

void cgpu_unmap_buffer_metal(CGPUBufferId buffer)
{
    CGPUBuffer_Metal* B = (CGPUBuffer_Metal*)buffer;
    CGPUBufferInfo* pInfo = (CGPUBufferInfo*)B->super.info;
    pInfo->cpu_mapped_address = CGPU_NULLPTR; 
}

void cgpu_free_buffer_metal(CGPUBufferId buffer)
{
    CGPUBuffer_Metal* B = (CGPUBuffer_Metal*)buffer;
    B->mtlBuffer = nil;
    cgpu_free(B);
}

CGPUBufferViewId cgpu_create_buffer_view_metal(CGPUDeviceId device, const struct CGPUBufferViewDescriptor* desc)
{
    CGPUDevice_Metal* D = (CGPUDevice_Metal*)device;
    const CGPUBuffer_Metal* B = (const CGPUBuffer_Metal*)desc->buffer;
    
    // Allocate memory for the buffer view structure and descriptor
    CGPUBufferView_Metal* BV = (CGPUBufferView_Metal*)cgpu_calloc(1, sizeof(CGPUBufferView_Metal) + sizeof(CGPUBufferViewDescriptor));
    CGPUBufferViewDescriptor* Info = (CGPUBufferViewDescriptor*)(BV + 1);
    BV->super.info = Info;
    
    // Copy the descriptor
    *Info = *desc;
    
    // Store reference to the underlying Metal buffer
    BV->mtlBuffer = B->mtlBuffer;
    
    // Calculate the actual offset and size
    const uint64_t BufferViewSize = desc->size ? desc->size : (B->super.info->size - desc->offset);
    const uint64_t BufferOffset = desc->offset;
    
    // Update the descriptor with actual values
    Info->offset = BufferOffset;
    Info->size = BufferViewSize;

    if (desc->view_usages & CGPU_BUFFER_VIEW_USAGE_UAV_TEXEL ||
        desc->view_usages & CGPU_BUFFER_VIEW_USAGE_SRV_TEXEL)
    {
        bool writable = (desc->view_usages & CGPU_BUFFER_VIEW_USAGE_UAV_TEXEL) != 0;
        const uint32_t bytesPerPixel = FormatUtil_BitSizeOfBlock(desc->texel.format) / 8;
        const uint32_t texelCount = (uint32_t)BufferViewSize / bytesPerPixel;
        MTLTextureDescriptor* texelDesc = [[MTLTextureDescriptor alloc] init];
        texelDesc.textureType = MTLTextureTypeTextureBuffer;
        texelDesc.pixelFormat = MetalUtil_TranslatePixelFormat(desc->texel.format);
        texelDesc.width = texelCount;
        texelDesc.height = 1;
        texelDesc.depth = 1;
        texelDesc.usage = writable ? MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite : MTLTextureUsageShaderRead;
        texelDesc.resourceOptions = B->mtlResourceOptions;
        const uint32_t bytesPerRow = texelCount * bytesPerPixel;
        BV->mtlTextureBuffer = [BV->mtlBuffer newTextureWithDescriptor:texelDesc 
                                                        offset: B->mOffset + desc->offset
                                                        bytesPerRow:bytesPerRow];
        [texelDesc release];
    }
    
    // Store the offset for Metal-specific operations
    BV->mOffset = B->mOffset + BufferOffset;
    
    return &BV->super;
}

void cgpu_free_buffer_view_metal(CGPUBufferViewId view)
{
    CGPUBufferView_Metal* BV = (CGPUBufferView_Metal*)view;
    
    // Clear the Metal buffer reference
    BV->mtlBuffer = nil;
    BV->mtlTextureBuffer = nil;
    
    // Free the buffer view structure
    cgpu_free(BV);
}

// Sampler APIs
static inline MTLSamplerAddressMode MetalUtil_TranslateAddressMode(ECGPUAddressMode mode) 
{
    switch (mode) {
        case CGPU_ADDRESS_MODE_MIRROR:
            return MTLSamplerAddressModeMirrorRepeat;
        case CGPU_ADDRESS_MODE_REPEAT:
            return MTLSamplerAddressModeRepeat;
        case CGPU_ADDRESS_MODE_CLAMP_TO_EDGE:
            return MTLSamplerAddressModeClampToEdge;
        case CGPU_ADDRESS_MODE_CLAMP_TO_BORDER:
            return MTLSamplerAddressModeClampToBorderColor;
        default:
            return MTLSamplerAddressModeClampToEdge;
    }
}

CGPUSamplerId cgpu_create_sampler_metal(CGPUDeviceId device, const struct CGPUSamplerDescriptor* desc)
{
    CGPUDevice_Metal* D = (CGPUDevice_Metal*)device;
    CGPUSampler_Metal* pSampler = cgpu_calloc(1, sizeof(CGPUSampler_Metal));
    pSampler->super.device = device;
    
    MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];
    samplerDesc.supportArgumentBuffers = true;
    
    // Set filter modes
    samplerDesc.minFilter = (desc->min_filter == CGPU_FILTER_TYPE_LINEAR) ? 
        MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest;
    samplerDesc.magFilter = (desc->mag_filter == CGPU_FILTER_TYPE_LINEAR) ? 
        MTLSamplerMinMagFilterLinear : MTLSamplerMinMagFilterNearest;
    samplerDesc.mipFilter = (desc->mipmap_mode == CGPU_MIPMAP_MODE_LINEAR) ? 
        MTLSamplerMipFilterLinear : MTLSamplerMipFilterNearest;
    
    // Set address modes
    samplerDesc.sAddressMode = MetalUtil_TranslateAddressMode(desc->address_u);
    samplerDesc.tAddressMode = MetalUtil_TranslateAddressMode(desc->address_v);
    samplerDesc.rAddressMode = MetalUtil_TranslateAddressMode(desc->address_w);
    
    // Set LOD parameters
    samplerDesc.lodMinClamp = 0.0f;
    samplerDesc.lodMaxClamp = (desc->mipmap_mode == CGPU_MIPMAP_MODE_LINEAR) ? FLT_MAX : 0.0f;
    
    // Set anisotropy
    if (desc->max_anisotropy > 0.0f) {
        samplerDesc.maxAnisotropy = (NSUInteger)cgpu_max(desc->max_anisotropy, 1.0f);
    } else {
        samplerDesc.maxAnisotropy = 1;
    }
    
    // Set compare function if needed
    if (desc->compare_func != CGPU_CMP_NEVER) {
        samplerDesc.compareFunction = (MTLCompareFunction)desc->compare_func;
    } else {
        samplerDesc.compareFunction = MTLCompareFunctionNever;
    }
    
    // Set border color (Metal only supports a limited set of border colors)
    samplerDesc.borderColor = MTLSamplerBorderColorTransparentBlack;
    
    // Create the sampler state
    pSampler->mtlSamplerState = [D->pDevice newSamplerStateWithDescriptor:samplerDesc];
    
    return &pSampler->super;
}

void cgpu_free_sampler_metal(CGPUSamplerId sampler)
{
    CGPUSampler_Metal* pSampler = (CGPUSampler_Metal*)sampler;
    pSampler->mtlSamplerState = nil;
    cgpu_free(pSampler);
}

// CMDs
void cgpu_cmd_begin_metal(CGPUCommandBufferId cmd) 
{
    CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)cmd;
    CGPUCommandPool_Metal* Pool = (CGPUCommandPool_Metal*)cmd->pool;
    
    if (CMD->mtlCommandBuffer)
        CMD->mtlCommandBuffer = nil;
    
    CMD->UtilEncoders.mtlBlitEncoder = nil;
    CMD->UtilEncoders.mtlASCommandEncoder = nil;
    CMD->renderEncoder.mtlRenderEncoder = nil;
    CMD->cmptEncoder.mtlComputeEncoder = nil;
    
    CGPUQueue_Metal* Queue = (CGPUQueue_Metal*)Pool->super.queue;
    CMD->mtlCommandBuffer = [Queue->mtlCommandQueue commandBuffer];
    CMD->mtlCommandBuffer.label = @"CommandBuffer";
}

void cgpu_cmd_transfer_buffer_to_buffer_metal(CGPUCommandBufferId cmd, const struct CGPUBufferToBufferTransfer* desc)
{
    CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)cmd;
    MetalUtil_FlushUtilEncoders(CMD, MTLUtilEncoderTypeAS);

    CMD->UtilEncoders.mtlBlitEncoder = CMD->UtilEncoders.mtlBlitEncoder ? CMD->UtilEncoders.mtlBlitEncoder : [CMD->mtlCommandBuffer blitCommandEncoder];

    CGPUBuffer_Metal* srcBuffer = (CGPUBuffer_Metal*)desc->src;
    CGPUBuffer_Metal* dstBuffer = (CGPUBuffer_Metal*)desc->dst;
    [CMD->UtilEncoders.mtlBlitEncoder copyFromBuffer: srcBuffer->mtlBuffer
        sourceOffset: srcBuffer->mOffset + desc->src_offset
        toBuffer: dstBuffer->mtlBuffer
        destinationOffset: dstBuffer->mOffset + desc->dst_offset
        size: desc->size];
}

void cgpu_cmd_transfer_buffer_to_texture_metal(CGPUCommandBufferId cmd, const struct CGPUBufferToTextureTransfer* desc)
{
    CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)cmd;
    CGPUBuffer_Metal* srcBuffer = (CGPUBuffer_Metal*)desc->src;
    CGPUTexture_Metal* dstTexture = (CGPUTexture_Metal*)desc->dst;
    
    // Check if destination texture is framebufferOnly
    if (dstTexture->pTexture.framebufferOnly)
    {
        // For framebufferOnly textures, we need to use a render pass
        MetalUtil_FlushUtilEncoders(CMD, MTLUtilEncoderTypeAS | MTLUtilEncoderTypeBlit);
        
        // Create render pass descriptor
        MTLRenderPassDescriptor* renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDesc.colorAttachments[0].texture = dstTexture->pTexture;
        renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionDontCare;
        renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
        
        // Create render encoder
        id<MTLRenderCommandEncoder> renderEncoder = [CMD->mtlCommandBuffer renderCommandEncoderWithDescriptor:renderPassDesc];
        renderEncoder.label = @"Blit Buffer to Texture";
        
        // Set viewport to cover the entire texture
        MTLViewport viewport = {
            .originX = 0,
            .originY = 0,
            .width = (double)dstTexture->super.info->width,
            .height = (double)dstTexture->super.info->height,
            .znear = 0.0,
            .zfar = 1.0
        };
        [renderEncoder setViewport:viewport];
        
        // Get the blit pipeline from device
        CGPUDevice_Metal* device = (CGPUDevice_Metal*)CMD->super.device;
        id<MTLRenderPipelineState> blitPipeline = MetalUtil_GetBlitPipeline(device, dstTexture->pTexture.pixelFormat);
        if (!blitPipeline) {
            cgpu_error("Failed to get blit pipeline for buffer to texture copy");
            [renderEncoder endEncoding];
            return;
        }
        [renderEncoder setRenderPipelineState:blitPipeline];
        
        // Set source buffer at vertex buffer binding 0
        [renderEncoder setVertexBuffer:srcBuffer->mtlBuffer offset:srcBuffer->mOffset + desc->src_offset atIndex:0];
        
        // Draw fullscreen triangle (3 vertices, no vertex buffer needed)
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
        
        // End encoding
        [renderEncoder endEncoding];
        return;
    }
    
    // For non-framebufferOnly textures, use blit encoder
    MetalUtil_FlushUtilEncoders(CMD, MTLUtilEncoderTypeAS);
    
    // Ensure we have a blit encoder
    CMD->UtilEncoders.mtlBlitEncoder = CMD->UtilEncoders.mtlBlitEncoder ? CMD->UtilEncoders.mtlBlitEncoder : [CMD->mtlCommandBuffer blitCommandEncoder];
    
    // Calculate texture dimensions for the copy
    const CGPUTextureInfo* dstInfo = desc->dst->info;
    
    // Get the mip level dimensions
    uint32_t mipWidth = dstInfo->width >> desc->dst_subresource.mip_level;
    uint32_t mipHeight = dstInfo->height >> desc->dst_subresource.mip_level;
    uint32_t mipDepth = dstInfo->depth >> desc->dst_subresource.mip_level;
    
    // Ensure minimum size of 1
    mipWidth = mipWidth ? mipWidth : 1;
    mipHeight = mipHeight ? mipHeight : 1;
    mipDepth = mipDepth ? mipDepth : 1;
    
    // For 2D textures, depth should be 1
    if (dstInfo->depth == 1)
        mipDepth = 1;
    
    // Calculate bytes per row and bytes per image for the texture format
    MTLPixelFormat pixelFormat = dstTexture->pTexture.pixelFormat;
    NSUInteger bytesPerPixel = 0;
    NSUInteger bytesPerRow = 0;
    NSUInteger bytesPerImage = 0;
    
    // Get bytes per pixel for the format
    // Use a simple lookup table for common formats
    switch (pixelFormat) {
        case MTLPixelFormatRGBA8Unorm:
        case MTLPixelFormatBGRA8Unorm:
        case MTLPixelFormatRGBA8Unorm_sRGB:
        case MTLPixelFormatBGRA8Unorm_sRGB:
            bytesPerPixel = 4;
            break;
        case MTLPixelFormatRGBA16Float:
            bytesPerPixel = 8;
            break;
        case MTLPixelFormatRGBA32Float:
            bytesPerPixel = 16;
            break;
        case MTLPixelFormatR8Unorm:
        case MTLPixelFormatR8Unorm_sRGB:
            bytesPerPixel = 1;
            break;
        case MTLPixelFormatRG8Unorm:
        case MTLPixelFormatRG8Unorm_sRGB:
            bytesPerPixel = 2;
            break;
        case MTLPixelFormatRGB10A2Unorm:
            bytesPerPixel = 4;
            break;
        case MTLPixelFormatR16Float:
            bytesPerPixel = 2;
            break;
        case MTLPixelFormatR32Float:
            bytesPerPixel = 4;
            break;
        default:
            // For compressed formats or other formats, we need to calculate based on block size
            // This is a simplified approach - in practice, you might need more sophisticated calculation
            bytesPerPixel = 4; // Default fallback
            break;
    }
    
    bytesPerRow = mipWidth * bytesPerPixel;
    bytesPerImage = bytesPerRow * mipHeight;
    
    // Copy buffer to texture for each layer
    for (uint32_t layer = 0; layer < desc->dst_subresource.layer_count; layer++)
    {
        MTLOrigin dstOrigin = MTLOriginMake(0, 0, 0);
        MTLSize dstSize = MTLSizeMake(mipWidth, mipHeight, mipDepth);
        
        uint32_t dstSlice = desc->dst_subresource.base_array_layer + layer;
        uint64_t bufferOffset = desc->src_offset + (layer * bytesPerImage);
        
        [CMD->UtilEncoders.mtlBlitEncoder copyFromBuffer:srcBuffer->mtlBuffer
                                            sourceOffset:srcBuffer->mOffset + bufferOffset
                                       sourceBytesPerRow:bytesPerRow
                                     sourceBytesPerImage:bytesPerImage
                                              sourceSize:dstSize
                                               toTexture:dstTexture->pTexture
                                        destinationSlice:dstSlice
                                        destinationLevel:desc->dst_subresource.mip_level
                                       destinationOrigin:dstOrigin];
    }
}

void cgpu_cmd_transfer_texture_to_texture_metal(CGPUCommandBufferId cmd, const struct CGPUTextureToTextureTransfer* desc)
{
    CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)cmd;
    CGPUTexture_Metal* srcTexture = (CGPUTexture_Metal*)desc->src;
    CGPUTexture_Metal* dstTexture = (CGPUTexture_Metal*)desc->dst;
    
    // Check if destination texture is framebufferOnly
    if (dstTexture->pTexture.framebufferOnly)
    {
        // For framebufferOnly textures, we need to use a render pass
        MetalUtil_FlushUtilEncoders(CMD, MTLUtilEncoderTypeAS | MTLUtilEncoderTypeBlit);
        
        // Create render pass descriptor
        MTLRenderPassDescriptor* renderPassDesc = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDesc.colorAttachments[0].texture = dstTexture->pTexture;
        renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionDontCare;
        renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
        
        // Create render encoder
        id<MTLRenderCommandEncoder> renderEncoder = [CMD->mtlCommandBuffer renderCommandEncoderWithDescriptor:renderPassDesc];
        renderEncoder.label = @"Blit Texture";
        
        // Set viewport to cover the entire texture
        MTLViewport viewport = {
            .originX = 0,
            .originY = 0,
            .width = (double)dstTexture->super.info->width,
            .height = (double)dstTexture->super.info->height,
            .znear = 0.0,
            .zfar = 1.0
        };
        [renderEncoder setViewport:viewport];
        
        // Get the blit pipeline from device
        CGPUDevice_Metal* device = (CGPUDevice_Metal*)CMD->super.device;
        id<MTLRenderPipelineState> blitPipeline = MetalUtil_GetBlitPipeline(device, dstTexture->pTexture.pixelFormat);
        if (!blitPipeline) {
            cgpu_error("Failed to get blit pipeline for texture copy");
            [renderEncoder endEncoding];
            return;
        }
        [renderEncoder setRenderPipelineState:blitPipeline];
        
        // Set source texture at fragment texture binding 0
        [renderEncoder setFragmentTexture:srcTexture->pTexture atIndex:0];
        
        // Get sampler and set at fragment sampler binding 0
        id<MTLSamplerState> sampler = MetalUtil_GetLinearSampler(device);
        [renderEncoder setFragmentSamplerState:sampler atIndex:0];
        
        // Draw fullscreen triangle (3 vertices, no vertex buffer needed)
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
        
        // End encoding
        [renderEncoder endEncoding];
        return;
    }
    
    // For non-framebufferOnly textures, use normal blit encoder
    MetalUtil_FlushUtilEncoders(CMD, MTLUtilEncoderTypeAS);
    
    // Ensure we have a blit encoder
    CMD->UtilEncoders.mtlBlitEncoder = CMD->UtilEncoders.mtlBlitEncoder ? CMD->UtilEncoders.mtlBlitEncoder : [CMD->mtlCommandBuffer blitCommandEncoder];
    
    // Calculate texture dimensions for the copy
    const CGPUTextureInfo* srcInfo = desc->src->info;
    const CGPUTextureInfo* dstInfo = desc->dst->info;
    
    // Get the mip level dimensions
    uint32_t mipWidth = srcInfo->width >> desc->src_subresource.mip_level;
    uint32_t mipHeight = srcInfo->height >> desc->src_subresource.mip_level;
    uint32_t mipDepth = srcInfo->depth >> desc->src_subresource.mip_level;
    
    // Ensure minimum size of 1
    mipWidth = mipWidth ? mipWidth : 1;
    mipHeight = mipHeight ? mipHeight : 1;
    mipDepth = mipDepth ? mipDepth : 1;
    
    // For 2D textures, depth should be 1
    if (srcInfo->depth == 1)
        mipDepth = 1;
    
    // Copy texture to texture
    for (uint32_t layer = 0; layer < desc->src_subresource.layer_count; layer++)
    {
        MTLOrigin srcOrigin = MTLOriginMake(0, 0, 0);
        MTLSize srcSize = MTLSizeMake(mipWidth, mipHeight, mipDepth);
        MTLOrigin dstOrigin = MTLOriginMake(0, 0, 0);
        
        uint32_t srcSlice = desc->src_subresource.base_array_layer + layer;
        uint32_t dstSlice = desc->dst_subresource.base_array_layer + layer;
        
        [CMD->UtilEncoders.mtlBlitEncoder copyFromTexture:srcTexture->pTexture
                                              sourceSlice:srcSlice
                                              sourceLevel:desc->src_subresource.mip_level
                                             sourceOrigin:srcOrigin
                                               sourceSize:srcSize
                                                toTexture:dstTexture->pTexture
                                         destinationSlice:dstSlice
                                         destinationLevel:desc->dst_subresource.mip_level
                                        destinationOrigin:dstOrigin];
    }
}

void cgpu_cmd_fill_buffer_metal(CGPUCommandBufferId cmd, CGPUBufferId buffer, const struct CGPUFillBufferDescriptor* desc)
{
    CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)cmd;
    CGPUBuffer_Metal* B = (CGPUBuffer_Metal*)buffer;
    
    MetalUtil_FlushUtilEncoders(CMD, MTLUtilEncoderTypeAS);
    
    // Ensure we have a blit encoder
    CMD->UtilEncoders.mtlBlitEncoder = CMD->UtilEncoders.mtlBlitEncoder ? CMD->UtilEncoders.mtlBlitEncoder : [CMD->mtlCommandBuffer blitCommandEncoder];
    
    // Fill buffer with pattern (4 bytes)
    [CMD->UtilEncoders.mtlBlitEncoder fillBuffer:B->mtlBuffer
                                           range:NSMakeRange(B->mOffset + desc->offset, sizeof(uint32_t))
                                           value:desc->value];
}

void cgpu_cmd_fill_buffer_n_metal(CGPUCommandBufferId cmd, CGPUBufferId buffer, const struct CGPUFillBufferDescriptor* desc, uint32_t count)
{
    CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)cmd;
    CGPUBuffer_Metal* B = (CGPUBuffer_Metal*)buffer;
    
    MetalUtil_FlushUtilEncoders(CMD, MTLUtilEncoderTypeAS);
    
    // Ensure we have a blit encoder
    CMD->UtilEncoders.mtlBlitEncoder = CMD->UtilEncoders.mtlBlitEncoder ? CMD->UtilEncoders.mtlBlitEncoder : [CMD->mtlCommandBuffer blitCommandEncoder];
    
    // Fill buffer with pattern (4 bytes * count)
    [CMD->UtilEncoders.mtlBlitEncoder fillBuffer:B->mtlBuffer
                                           range:NSMakeRange(B->mOffset + desc->offset, sizeof(uint32_t) * count)
                                           value:desc->value];
}

void cgpu_cmd_resource_barrier_metal(CGPUCommandBufferId cmd, const struct CGPUResourceBarrierDescriptor* desc)
{
    // TODO: Implement resource barrier with updateFence
}

void cgpu_cmd_begin_event_metal(CGPUCommandBufferId cmd, const CGPUEventInfo* event)
{
    if (!event || !event->name) return;
    
    CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)cmd;
    NSString* label = [NSString stringWithUTF8String:(const char*)event->name];
    
    // Push debug group for all active encoders
    if (CMD->renderEncoder.mtlRenderEncoder)
        [CMD->renderEncoder.mtlRenderEncoder pushDebugGroup:label];
    else if (CMD->cmptEncoder.mtlComputeEncoder)
        [CMD->cmptEncoder.mtlComputeEncoder pushDebugGroup:label];
    else if (CMD->UtilEncoders.mtlBlitEncoder)
        [CMD->UtilEncoders.mtlBlitEncoder pushDebugGroup:label];
}

void cgpu_cmd_end_event_metal(CGPUCommandBufferId cmd)
{
    CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)cmd;
    
    // Pop debug group for all active encoders
    if (CMD->renderEncoder.mtlRenderEncoder)
        [CMD->renderEncoder.mtlRenderEncoder popDebugGroup];
    else if (CMD->cmptEncoder.mtlComputeEncoder)
        [CMD->cmptEncoder.mtlComputeEncoder popDebugGroup];
    else if (CMD->UtilEncoders.mtlBlitEncoder)
        [CMD->UtilEncoders.mtlBlitEncoder popDebugGroup];
}

void cgpu_cmd_end_metal(CGPUCommandBufferId cmd) {  }

// Query CMDs
void cgpu_cmd_begin_query_metal(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc)
{
    // Metal doesn't have direct begin/end query like D3D12/Vulkan
    // For timestamp queries, we would use addCompletedHandler or similar
    // For now, this is a placeholder implementation
}

void cgpu_cmd_end_query_metal(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, const struct CGPUQueryDescriptor* desc)
{
    // Metal doesn't have direct begin/end query like D3D12/Vulkan
    // For timestamp queries, we would use addCompletedHandler or similar
    // For now, this is a placeholder implementation
}

void cgpu_cmd_reset_query_pool_metal(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, uint32_t start_query, uint32_t query_count)
{
    // Metal doesn't require explicit query pool reset
    // This is a no-op for Metal backend
}

void cgpu_cmd_resolve_query_metal(CGPUCommandBufferId cmd, CGPUQueryPoolId pool, CGPUBufferId readback, uint32_t start_query, uint32_t query_count)
{
    // Metal query resolution would be handled differently
    // For timestamp queries, data would be available through completion handlers
    // For now, this is a placeholder implementation
}

// Compute CMDs
CGPUComputePassEncoderId cgpu_cmd_begin_compute_pass_metal(CGPUCommandBufferId cmd, const struct CGPUComputePassDescriptor* desc)
{
    CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)cmd;
    MetalUtil_FlushUtilEncoders(CMD, MTLUtilEncoderTypeAS | MTLUtilEncoderTypeBlit);

    CGPUComputePassEncoder_Metal* CE = &CMD->cmptEncoder;
    CE->mtlComputeEncoder = nil;
    CE->super.device = CMD->super.device;
    CE->mtlComputeEncoder = [CMD->mtlCommandBuffer computeCommandEncoder];
    CE->threadgroupSize = MTLSizeMake(0, 0, 0); // 初始化为0，表示未设置
    CMD->cmptEncoder = *CE;
    return &CE->super;
}

void cgpu_compute_encoder_bind_descriptor_set_metal(CGPUComputePassEncoderId encoder, CGPUDescriptorSetId set)
{
    CGPUComputePassEncoder_Metal* CE = (CGPUComputePassEncoder_Metal*)encoder;
    CGPUDescriptorSet_Metal* DS = (CGPUDescriptorSet_Metal*)set;
    if (DS->mtlArgumentBuffer)
    {      
        CGPUBuffer_Metal* B = (CGPUBuffer_Metal*)DS->mtlArgumentBuffer;
        [CE->mtlComputeEncoder setBuffer:B->mtlBuffer offset:B->mOffset atIndex:DS->super.index];

        uint32_t ReadCount = 0, ReadWriteCount = 0;
        for (uint32_t i = 0; i < DS->mtlBindSlotCount; i++)
        {
            BindSlot_Metal* slot = DS->mtlBindSlots + i;
            if (slot->mtlUsage == MTLResourceUsageRead)
            {
                DS->mtlReadArgsCache[ReadCount] = slot->mtlResource;
                ReadCount++;
            }
            else if (slot->mtlUsage == (MTLResourceUsageRead | MTLResourceUsageWrite))
            {
                DS->mtlReadWriteArgsCache[ReadWriteCount] = slot->mtlResource;
                ReadWriteCount++;
            }
            else if (slot->mtlUsage == MTLResourceUsageWrite)
            {
                cgpu_assert(0 && "Unexpected MTLResourceUsageWrite!");
            }
        }
        if (DS->pBoundAS != nil)
            [CE->mtlComputeEncoder useResources:DS->pBoundAS->asTop.mtlBottomASRefs count: DS->pBoundAS->asTop.mtlBottomASRefsCount usage:MTLResourceUsageRead];
        if (ReadCount > 0)
            [CE->mtlComputeEncoder useResources:DS->mtlReadArgsCache count: ReadCount usage:MTLResourceUsageRead];
        if (ReadWriteCount > 0)
            [CE->mtlComputeEncoder useResources:DS->mtlReadWriteArgsCache count: ReadWriteCount usage:MTLResourceUsageRead | MTLResourceUsageWrite];
    }
}

void cgpu_compute_encoder_bind_descriptor_buffer_metal(CGPUComputePassEncoderId encoder, CGPUDescriptorBufferId buffer, const char8_t* set_name)
{
    CGPUDescriptorBuffer_Metal* DB = (CGPUDescriptorBuffer_Metal*)buffer;
    CGPUComputePassEncoder_Metal* CE = (CGPUComputePassEncoder_Metal*)encoder;
    CGPUComputePipeline_Metal* CP = (CGPUComputePipeline_Metal*)CE->pBoundPipeline;
    CGPURootSignature_Metal* RS = (CGPURootSignature_Metal*)CP->super.root_signature;
    uint64_t hash = skr_hash_of(set_name, strlen(set_name), SKR_DEFAULT_HASH_SEED);
    uint32_t set_index = UINT32_MAX;
    CGPUViewUsages view_usages = CGPU_VIEW_USAGE_NONE;
    for (uint32_t i = 0; i < RS->super.table_count; i++)
    {
        CGPUParameterTable* pTable = RS->super.tables + i;
        for (uint32_t j = 0; j < pTable->resources_count; j++)
        {
            if (pTable->resources[j].name_hash == hash && 
                (strcmp(pTable->resources[j].name, set_name) == 0))
            {
                set_index = pTable->resources[j].set;
                view_usages = pTable->resources[j].view_usages;
            }
        }
    }
    if (set_index != UINT32_MAX)
    {
        CGPUBuffer_Metal* B = (CGPUBuffer_Metal*)DB->mtlArgumentBuffer;
        uint64_t GPUAddress = B->mtlBuffer.gpuAddress + B->mOffset;
        [CE->mtlComputeEncoder setBytes: &GPUAddress
                    length: sizeof(uint64_t)
                    atIndex:set_index];

        MTLResourceUsage usages = MTLResourceUsageRead;
        if (view_usages & CGPU_BUFFER_VIEW_USAGE_UAV_STRUCTURED ||
            view_usages & CGPU_BUFFER_VIEW_USAGE_UAV_RAW ||
            view_usages & CGPU_BUFFER_VIEW_USAGE_UAV_TEXEL ||
            view_usages & CGPU_TEXTURE_VIEW_USAGE_RTV_DSV ||
            view_usages & CGPU_TEXTURE_VIEW_USAGE_UAV)
        {
            usages = MTLResourceUsageRead | MTLResourceUsageWrite;
        }
        
        [CE->mtlComputeEncoder useResource:B->mtlBuffer usage:MTLResourceUsageRead];
        for (uint32_t i = 0; i < DB->super.size; i++)
        {
            if (DB->mtlArgsCache[i] != nil)
                [CE->mtlComputeEncoder useResource:DB->mtlArgsCache[i] usage:usages];
        }
    }
}

void cgpu_compute_encoder_push_constants_metal(CGPUComputePassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data)
{
    CGPUComputePassEncoder_Metal* CE = (CGPUComputePassEncoder_Metal*)encoder;
    CGPURootSignature_Metal* RS = (CGPURootSignature_Metal*)rs;
    
    // Find the push constant by name
    skr_hash name_hash = name ? skr_hash_of(name, strlen((const char*)name), SKR_DEFAULT_HASH_SEED) : 0;
    for (uint32_t i = 0; i < RS->super.push_constant_count; i++)
    {
        CGPUShaderResource* push_const = &RS->super.push_constants[0];
        
        // Match by name hash if name is provided, otherwise use the first push constant
        if (!name || (push_const->name_hash == name_hash && 
            strcmp((const char*)push_const->name, (const char*)name) == 0))
        {
            // In Metal, push constants are set using setBytes at a specific buffer index
            // The buffer index should be stored in the binding field during root signature creation
            [CE->mtlComputeEncoder setBytes:data 
                                    length:push_const->size 
                                    atIndex:push_const->set];
        }
    }
}

void cgpu_compute_encoder_bind_pipeline_metal(CGPUComputePassEncoderId encoder, CGPUComputePipelineId pipeline)
{
    CGPUComputePassEncoder_Metal* CE = (CGPUComputePassEncoder_Metal*)encoder;
    CGPUComputePipeline_Metal* CP = (CGPUComputePipeline_Metal*)pipeline;
    [CE->mtlComputeEncoder setComputePipelineState:CP->mtlPipelineState];
    CE->pBoundPipeline = CP;

    // 设置静态采样器到ArgBuffer并绑定
    CGPURootSignature_Metal* RS = (CGPURootSignature_Metal*)CP->super.root_signature;
    if (RS && RS->staticSamplerCount > 0 && RS->staticSamplerSets) {
        // 先设置静态采样器数据到ArgBuffer
        for (uint32_t i = 0; i < RS->staticSamplerCount; i++) {
            CGPUStaticSampler_Metal* staticSampler = &RS->staticSamplers[i];
            if (staticSampler->setIndex != UINT32_MAX && staticSampler->resourceOffset != UINT32_MAX) {
                // 找到对应的ArgBuffer
                for (uint32_t j = 0; j < RS->staticSamplerSetCount; j++) {
                    if (RS->staticSamplerSets[j].setIndex == staticSampler->setIndex) {
                        uint8_t* pBuf = RS->staticSamplerSets[j].argBuffer.contents;
                        uint8_t* pArg = pBuf + staticSampler->resourceOffset;
                        struct SAMPLER ARG = { staticSampler->mtlSamplerState.gpuResourceID };
                        memcpy(pArg, &ARG, sizeof(ARG));
                        break;
                    }
                }
            }
        }
        
        // 绑定所有静态采样器ArgBuffer
        for (uint32_t i = 0; i < RS->staticSamplerSetCount; i++) {
            [CE->mtlComputeEncoder setBuffer:RS->staticSamplerSets[i].argBuffer 
                                      offset:0 
                                     atIndex:RS->staticSamplerSets[i].setIndex];
        }
    }
}

void cgpu_compute_encoder_set_threadgroup_size_metal(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z)
{
    CGPUComputePassEncoder_Metal* CE = (CGPUComputePassEncoder_Metal*)encoder;
    CE->threadgroupSize = MTLSizeMake(X, Y, Z);
}

void cgpu_compute_encoder_dispatch_metal(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z)
{
    CGPUComputePassEncoder_Metal* CE = (CGPUComputePassEncoder_Metal*)encoder;
    
    // 使用设置的threadgroup size，如果没有设置则使用默认值
    MTLSize threadsPerGroup = CE->threadgroupSize;
    if (threadsPerGroup.width == 0 || threadsPerGroup.height == 0 || threadsPerGroup.depth == 0) {
        threadsPerGroup = MTLSizeMake(32, 32, 1); // 默认值
    }
    
    // 计算threadgroup数量，使用向上取整
    uint32_t groupCountX = (X + threadsPerGroup.width - 1) / threadsPerGroup.width;
    uint32_t groupCountY = (Y + threadsPerGroup.height - 1) / threadsPerGroup.height;
    uint32_t groupCountZ = (Z + threadsPerGroup.depth - 1) / threadsPerGroup.depth;
    
    MTLSize threadgroupCount = MTLSizeMake(groupCountX, groupCountY, groupCountZ);
    [CE->mtlComputeEncoder dispatchThreadgroups:threadgroupCount threadsPerThreadgroup:threadsPerGroup];
}

void cgpu_cmd_end_compute_pass_metal(CGPUCommandBufferId cmd, CGPUComputePassEncoderId encoder)
{
    CGPUCommandBuffer_Metal* MB = (CGPUCommandBuffer_Metal*)cmd;
    CGPUComputePassEncoder_Metal* CE = (CGPUComputePassEncoder_Metal*)encoder;
    [CE->mtlComputeEncoder endEncoding];
    CE->mtlComputeEncoder = nil;
    MB->cmptEncoder.mtlComputeEncoder = nil;
    CE->pBoundPipeline = nil;
}

void cgpu_render_encoder_push_constants_metal(CGPURenderPassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data)
{
    CGPURenderPassEncoder_Metal* RE = (CGPURenderPassEncoder_Metal*)encoder;
    CGPURootSignature_Metal* RS = (CGPURootSignature_Metal*)rs;
    
    // Find the push constant by name
    skr_hash name_hash = name ? skr_hash_of(name, strlen((const char*)name), SKR_DEFAULT_HASH_SEED) : 0;
    for (uint32_t i = 0; i < RS->super.push_constant_count; i++)
    {
        CGPUShaderResource* push_const = &RS->super.push_constants[i];
        
        // Match by name hash if name is provided, otherwise use the first push constant
        if (!name || (push_const->name_hash == name_hash && 
            strcmp((const char*)push_const->name, (const char*)name) == 0))
        {
            // In Metal, push constants are set using setBytes at a specific buffer index
            // The buffer index should be stored in the set field during root signature creation
            [RE->mtlRenderEncoder setVertexBytes:data 
                                          length:push_const->size 
                                         atIndex:push_const->set];
            [RE->mtlRenderEncoder setFragmentBytes:data 
                                            length:push_const->size 
                                           atIndex:push_const->set];
            break;
        }
    }
}