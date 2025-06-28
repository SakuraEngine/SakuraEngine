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
    .create_descriptor_set = &cgpu_create_descriptor_set_metal,
    .update_descriptor_set = &cgpu_update_descriptor_set_metal,
    .free_descriptor_set = &cgpu_free_descriptor_set_metal,
    .create_compute_pipeline = &cgpu_create_compute_pipeline_metal,
    .free_compute_pipeline = &cgpu_free_compute_pipeline_metal,
    .create_query_pool = &cgpu_create_query_pool_metal,
    .free_query_pool = &cgpu_free_query_pool_metal,

    // Queue APIs
    .get_queue = &cgpu_get_queue_metal,
    .submit_queue = &cgpu_submit_queue_metal,
    .wait_queue_idle = &cgpu_wait_queue_idle_metal,
    .free_queue = &cgpu_free_queue_metal,

    // Command APIs
    .create_command_pool = &cgpu_create_command_pool_metal,
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

    // CMDs
    .cmd_begin = &cgpu_cmd_begin_metal,
    .cmd_transfer_buffer_to_buffer = &cgpu_cmd_transfer_buffer_to_buffer_metal,
    .cmd_resource_barrier = &cgpu_cmd_resource_barrier_metal,
    .cmd_begin_query = &cgpu_cmd_begin_query_metal,
    .cmd_end_query = &cgpu_cmd_end_query_metal,
    .cmd_reset_query_pool = &cgpu_cmd_reset_query_pool_metal,
    .cmd_resolve_query = &cgpu_cmd_resolve_query_metal,
    .cmd_end = &cgpu_cmd_end_metal,

    // Compute CMDs
    .cmd_begin_compute_pass = &cgpu_cmd_begin_compute_pass_metal,
    .compute_encoder_bind_descriptor_set = &cgpu_compute_encoder_bind_descriptor_set_metal,
    .compute_encoder_push_constants = &cgpu_compute_encoder_push_constants_metal,
    .compute_encoder_bind_pipeline = &cgpu_compute_encoder_bind_pipeline_metal,
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
    CGPUFence_Metal* F = (CGPUFence_Metal*)cgpu_calloc(1, sizeof(CGPUFence_Metal));
    F->sysSemaphore = dispatch_semaphore_create(0);
    return &F->super;
}

ECGPUFenceStatus cgpu_query_fence_status_metal(CGPUFenceId fence)
{
    CGPUFence_Metal* F = (CGPUFence_Metal*)fence;
    long status = dispatch_semaphore_wait(F->sysSemaphore, DISPATCH_TIME_NOW);
    return (status == 0) ? CGPU_FENCE_STATUS_COMPLETE : CGPU_FENCE_STATUS_INCOMPLETE;
}

void cgpu_free_fence_metal(CGPUFenceId fence)
{
    CGPUFence_Metal* F = (CGPUFence_Metal*)fence;
    F->sysSemaphore = nil;
    cgpu_free(F);
}

void cgpu_wait_fences_metal(const CGPUFenceId* fences, uint32_t fence_count)
{
    for (uint32_t i = 0; i < fence_count; ++i)
    {
        CGPUFence_Metal* F = (CGPUFence_Metal*)fences[i];
        dispatch_semaphore_wait(F->sysSemaphore, DISPATCH_TIME_FOREVER);
    }
}

CGPUSemaphoreId cgpu_create_semaphore_metal(CGPUDeviceId device)
{
    CGPUDevice_Metal* D = (CGPUDevice_Metal*)device;
    CGPUSemaphore_Metal* MS = (CGPUSemaphore_Metal*)cgpu_calloc(1, sizeof(CGPUSemaphore_Metal));
    MS->mtlSemaphore = [D->pDevice newEvent];
    return &MS->super;
}

void cgpu_free_semaphore_metal(CGPUSemaphoreId semaphore)
{
    CGPUSemaphore_Metal* MS = (CGPUSemaphore_Metal*)semaphore;
    MS->mtlSemaphore = nil;
    cgpu_free(MS);
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
    RS->super.table_count = reflection.bindings.count;
    RS->super.tables = cgpu_calloc(RS->super.table_count, sizeof(CGPUParameterTable));
    for (uint32_t i = 0; i < reflection.bindings.count; i++)
    {
        CGPUParameterTable* table = RS->super.tables + i;
        table->metal.arg_buf_size = 0;
        if (reflection.bindings[i].type == MTLBindingTypeBuffer)
        {
            id<MTLBufferBinding> SRT = (id<MTLBufferBinding>)reflection.bindings[i];
            MTLStructType* SRTLayout = SRT.bufferStructType;
            table->metal.arg_buf_size = SRT.bufferDataSize;
            table->set_index = SRT.index;
            table->resources_count = SRTLayout.members.count;
            table->resources = cgpu_calloc(table->resources_count, sizeof(CGPUShaderResource));
            for (uint32_t j = 0; j < SRTLayout.members.count; j++)
            {
                MetalUtil_GetShaderResourceType(SRT.index, SRTLayout.members[j], table->resources + j);
            }
        }
        else
        {
            SKR_ASSERT(false && "Unsupported binding type in MTLBindingAccess");
        }
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

CGPUDescriptorSetId cgpu_create_descriptor_set_metal(CGPUDeviceId device, const struct CGPUDescriptorSetDescriptor* desc)
{
    CGPUDescriptorSet_Metal* DS = (CGPUDescriptorSet_Metal*)cgpu_calloc(1, sizeof(CGPUDescriptorSet_Metal));
    CGPURootSignature_Metal* RS = (CGPURootSignature_Metal*)desc->root_signature;
    CGPUParameterTable* set_table = RS->super.tables + desc->set_index;
    DS->mtlBindSlots = cgpu_calloc(set_table->resources_count, sizeof(BindSlot_Metal));
    DS->mtlReadArgsCache = (__strong id*)cgpu_calloc(set_table->resources_count, sizeof(id));
    DS->mtlReadWriteArgsCache = (__strong id*)cgpu_calloc(set_table->resources_count, sizeof(id));
    if (set_table->resources_count > 0)
    {
        CGPUBufferDescriptor buffer_desc = {};
        buffer_desc.size = set_table->metal.arg_buf_size;
        buffer_desc.name = "ArgumentBuffer";
        buffer_desc.descriptors = CGPU_RESOURCE_TYPE_BUFFER;
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
        uint8_t* pArg = ArgumentBuffer->mtlBuffer.contents + ArgumentBuffer->mOffset;
        for (uint32_t i = 0; i < count; i++)
        {
            const CGPUDescriptorData* data = datas + i;

            // find correct binding slot
            uint32_t binding_index = data->binding;
            CGPUShaderResource* resource = CGPU_NULLPTR;
            if (data->name != CGPU_NULLPTR)
            {
                size_t name_hash = cgpu_name_hash(data->name, strlen((const char*)data->name));
                for (uint32_t j = 0; j < set_table->resources_count; j++)
                {
                    CGPUShaderResource* temp_resource = set_table->resources + j;
                    if (temp_resource->name_hash == name_hash && 
                        strcmp((const char*)data->name, (const char*)temp_resource->name) == 0)
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
            pArg += resource->offset;

            // fill argument
            typedef struct BUFFER { uint64_t ptr; uint64_t size; } BUFFER;
            typedef struct CBUFFER { uint64_t ptr; } CBUFFER;
            // typedef struct TEXTURE { MTLResourceID id; } TEXTURE;
            // typedef struct SAMPLER { MTLResourceID id; } SAMPLER;
            typedef struct TLAS { MTLResourceID id; } TLAS;
            switch (data->binding_type)
            {
                case CGPU_RESOURCE_TYPE_BUFFER:
                case CGPU_RESOURCE_TYPE_RW_BUFFER:
                {
                    const MTLResourceUsage usage = (resource->type == CGPU_RESOURCE_TYPE_RW_BUFFER) ? MTLResourceUsageRead | MTLResourceUsageWrite : MTLResourceUsageRead;
                    if (resource->size != UINT32_MAX)
                    {
                        for (uint32_t j = 0; j < data->count; j++)
                        {
                            BUFFER* pBufferArg = ((BUFFER*)pArg) + j;
                            const CGPUBuffer_Metal* B = (const CGPUBuffer_Metal*)data->buffers[j];
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
                break;
                case CGPU_RESOURCE_TYPE_UNIFORM_BUFFER:
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
                break;
                case CGPU_RESOURCE_TYPE_TEXTURE:
                {
                    SKR_UNIMPLEMENTED_FUNCTION();
                }
                break;
                case CGPU_RESOURCE_TYPE_ACCELERATION_STRUCTURE:
                {
                    SKR_ASSERT(resource->size == 1 && "TLAS must have size 1");
                    TLAS* pTLAS = (TLAS*)pArg;
                    const CGPUAccelerationStructure_Metal* AS = (const CGPUAccelerationStructure_Metal*)data->acceleration_structures[0];
                    struct TLAS ARG = { 
                        AS->mtlAS.gpuResourceID
                    };
                    MetalUtil_DSBindResourceAtIndex(DS, binding_index, AS->mtlAS, MTLResourceUsageRead);
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
    MQ->mtlCommandQueue = MD->ppMtlQueues[type][index];
#if defined(ENABLE_FENCES)
    if (@available(macOS 10.13, iOS 10.0, *))
    {
        MQ->mtlQueueFence = [MD->pDevice newFence];
    }
#endif
    MQ->mBarrierFlags = 0;
    return &MQ->super;
}

void cgpu_submit_queue_metal(CGPUQueueId queue, const struct CGPUQueueSubmitDescriptor* desc)
{
    CGPUQueue_Metal* Q = (CGPUQueue_Metal*)queue;
    SKR_ASSERT(desc->signal_semaphores == CGPU_NULLPTR && "IMPLEMENT THIS PLEASE!");
    SKR_ASSERT(desc->wait_semaphores == CGPU_NULLPTR && "IMPLEMENT THIS PLEASE!");
    SKR_ASSERT(desc->signal_fence == CGPU_NULLPTR && "IMPLEMENT THIS PLEASE!");

    @autoreleasepool
    {
        // signal semaphores
        for (uint32_t i = 0; i < desc->signal_semaphore_count; i++)
        {
            CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)desc->cmds[desc->cmds_count - 1];
            CGPUSemaphore_Metal* Semaphore = (CGPUSemaphore_Metal*)desc->signal_semaphores[i];
            [CMD->mtlCommandBuffer encodeSignalEvent:Semaphore->mtlSemaphore value:++Semaphore->value];
        }

        // signal fence
        __block SAtomicU32 commandsFinished = 0;
        __block const CGPUFence_Metal* signalFence = (const CGPUFence_Metal*)desc->signal_fence;
        for (uint32_t i = 0; i < desc->cmds_count; i++)
        {
            CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)desc->cmds[i];
            [CMD->mtlCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
                uint32_t handlersCalled = 1u + skr_atomic_fetch_add(&commandsFinished, 1);
                if (signalFence && (handlersCalled == desc->cmds_count))
                {
                    dispatch_semaphore_signal(signalFence->sysSemaphore);
                }
                if (buffer.error != nil)
                    cgpu_error("Failed to execute commands with error (%s)", [buffer.error.description UTF8String]);
            }];
        }

        // wait semaphores
        if (desc->wait_semaphore_count)
        {
            id<MTLCommandBuffer> waitCommandBuffer = [Q->mtlCommandQueue commandBufferWithUnretainedReferences];
            waitCommandBuffer.label = @"WAIT_COMMAND_BUFFER";
            for (uint32_t i = 0; i < desc->wait_semaphore_count; i++)
            {
                CGPUSemaphore_Metal* Semaphore = (CGPUSemaphore_Metal*)desc->wait_semaphores[i];
                // Wait for the semaphore to be signaled
                [waitCommandBuffer encodeWaitForEvent:Semaphore->mtlSemaphore value:Semaphore->value];
            }
            [waitCommandBuffer commit];
        }

        // commit
        for (uint32_t i = 0; i < desc->cmds_count; i++)
        {
            CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)desc->cmds[desc->cmds_count - 1];
            MetalUtil_FlushUtilEncoders(CMD, MTLUtilEncoderTypeAS | MTLUtilEncoderTypeBlit);
            [CMD->mtlCommandBuffer commit];
        }
    }
}

void cgpu_wait_queue_idle_metal(CGPUQueueId queue)
{
    CGPUQueue_Metal* Q = (CGPUQueue_Metal*)queue;
    id<MTLCommandBuffer> waitCmdBuf = [Q->mtlCommandQueue commandBufferWithUnretainedReferences];
    [waitCmdBuf commit];
    [waitCmdBuf waitUntilCompleted];
    waitCmdBuf = nil;
}

void cgpu_free_queue_metal(CGPUQueueId queue)
{
    CGPUQueue_Metal* Q = (CGPUQueue_Metal*)queue;
    Q->mtlCommandQueue = nil;
#if defined(ENABLE_FENCES)
    if (@available(macOS 10.13, iOS 10.0, *))
    {
        Q->mtlQueueFence = nil;
    }
#endif
}

// Command APIs
CGPUCommandPoolId cgpu_create_command_pool_metal(CGPUQueueId queue, const CGPUCommandPoolDescriptor* desc)
{
    CGPUCommandPool_Metal* PQ = (CGPUCommandPool_Metal*)cgpu_calloc(1, sizeof(CGPUCommandPool_Metal));
    return &PQ->super;
}

CGPUCommandBufferId cgpu_create_command_buffer_metal(CGPUCommandPoolId pool, const struct CGPUCommandBufferDescriptor* desc)
{
    CGPUCommandBuffer_Metal* MB = (CGPUCommandBuffer_Metal*)cgpu_calloc(1, sizeof(CGPUCommandBuffer_Metal));
    CGPUQueue_Metal* MQ = (CGPUQueue_Metal*)pool->queue;
    MB->mtlCommandBuffer = [MQ->mtlCommandQueue commandBuffer];
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

    if (desc->descriptors & CGPU_RESOURCE_TYPE_UNIFORM_BUFFER)
        pInfo->size = CGPU_ALIGN(pInfo->size, A->adapter_detail.uniform_buffer_alignment);
    
    MemoryType memoryType = MetalUtil_MemoryUsageToMemoryType(desc->memory_usage);
    MTLResourceOptions options = MetalUtil_MemoryTypeToResourceOptions(memoryType);
    
    // TODO: USE MTL-VMA TO OPTIMIZE
    B->mtlBuffer = [D->pDevice newBufferWithLength:pInfo->size options:options];
    B->mOffset = 0;

    if (desc->flags & CGPU_BCF_PERSISTENT_MAP_BIT)
        pInfo->cpu_mapped_address = B->mtlBuffer.contents + B->mOffset;

    pInfo->descriptors = desc->descriptors;
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

// CMDs
void cgpu_cmd_begin_metal(CGPUCommandBufferId cmd) {  }

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

void cgpu_cmd_resource_barrier_metal(CGPUCommandBufferId cmd, const struct CGPUResourceBarrierDescriptor* desc)
{
    // TODO: Implement resource barrier with updateFence
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
    memset(CE, 0, sizeof(CGPUComputePassEncoder_Metal));
    CE->super.device = CMD->super.device;
    CE->mtlComputeEncoder = [CMD->mtlCommandBuffer computeCommandEncoder];
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
         
        // USE RESOURCE
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
                DS->mtlReadWriteArgsCache[ReadCount] = slot->mtlResource;
                ReadWriteCount++;
            }
            else if (slot->mtlUsage == MTLResourceUsageWrite)
            {
                cgpu_assert(0 && "Unexpected MTLResourceUsageWrite!");
            }
        }
        if (ReadCount > 0)
            [CE->mtlComputeEncoder useResources:DS->mtlReadArgsCache count: ReadCount usage:MTLResourceUsageRead];
        if (ReadWriteCount > 0)
            [CE->mtlComputeEncoder useResources:DS->mtlReadWriteArgsCache count: ReadWriteCount usage:MTLResourceUsageRead | MTLResourceUsageWrite];
    }
}

void cgpu_compute_encoder_push_constants_metal(CGPUComputePassEncoderId encoder, CGPURootSignatureId rs, const char8_t* name, const void* data)
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

void cgpu_compute_encoder_bind_pipeline_metal(CGPUComputePassEncoderId encoder, CGPUComputePipelineId pipeline)
{
    CGPUComputePassEncoder_Metal* CE = (CGPUComputePassEncoder_Metal*)encoder;
    CGPUComputePipeline_Metal* CP = (CGPUComputePipeline_Metal*)pipeline;
    [CE->mtlComputeEncoder setComputePipelineState:CP->mtlPipelineState];
}

void cgpu_compute_encoder_dispatch_metal(CGPUComputePassEncoderId encoder, uint32_t X, uint32_t Y, uint32_t Z)
{
    CGPUComputePassEncoder_Metal* CE = (CGPUComputePassEncoder_Metal*)encoder;
    MTLSize threadgroupCount = MTLSizeMake(X, Y, Z);
    // TODO: THREAD GROUP SIZE
    MTLSize threadsPerGroup = MTLSizeMake(32, 32, 1);
    [CE->mtlComputeEncoder dispatchThreadgroups:threadgroupCount threadsPerThreadgroup:threadsPerGroup];
}

void cgpu_cmd_end_compute_pass_metal(CGPUCommandBufferId cmd, CGPUComputePassEncoderId encoder)
{
    CGPUCommandBuffer_Metal* MB = (CGPUCommandBuffer_Metal*)cmd;
    CGPUComputePassEncoder_Metal* CE = (CGPUComputePassEncoder_Metal*)encoder;
    [CE->mtlComputeEncoder endEncoding];
    CE->mtlComputeEncoder = nil;
    MB->cmptEncoder.mtlComputeEncoder = nil;
}