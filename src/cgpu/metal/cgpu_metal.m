#include "metal_utils.h"
#include "cgpu/backend/metal/cgpu_metal.h"
#include "cgpu/backend/metal/cgpu_metal_types.h"

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
    .free_device = &cgpu_free_device_metal,

    // API Objects APIs
    .create_fence = &cgpu_create_fence_metal,
    .free_fence = &cgpu_free_fence_metal,

    // Queue APIs
    .get_queue = &cgpu_get_queue_metal,
    .submit_queue = &cgpu_submit_queue_metal,
    .wait_queue_idle = &cgpu_wait_queue_idle_metal,
    .free_queue = &cgpu_free_queue_metal,

    // Command APIs
    .create_command_pool = &cgpu_create_command_pool_metal,
    .create_command_buffer = &cgpu_create_command_buffer_metal,
    .free_command_buffer = &cgpu_free_command_buffer_metal,
    .free_command_pool = &cgpu_free_command_pool_metal
};

const CGPUProcTable* CGPU_MetalProcTable()
{
    return &tbl_metal;
}

NSArray<id<MTLDevice>>* MetalUtil_GetAvailableMTLDeviceArray();

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
        MA->device.ppMtlQueues[type] = cgpu_calloc(queueGroup->queue_count, sizeof(id<MTLCommandQueue>));
        MA->device.pMtlQueueCounts[type] = queueGroup->queue_count;
        for (uint32_t j = 0u; j < queueGroup->queue_count; j++)
        {
            MA->device.ppMtlQueues[type][j] = [MA->device.pDevice newCommandQueueWithMaxCommandBufferCount:512];
        }
    }
    return &MA->device.super;
}

void cgpu_free_device_metal(CGPUDeviceId device)
{
    CGPUDevice_Metal* MD = (CGPUDevice_Metal*)device;
    for (uint32_t i = 0; i < CGPU_QUEUE_TYPE_COUNT; i++)
    {
        if (MD->ppMtlQueues[i] != NULL && MD->pMtlQueueCounts[i] != 0)
        {
            for (uint32_t j = 0; j < MD->pMtlQueueCounts[i]; j++)
            {
                MD->ppMtlQueues[i][j] = nil;
            }
            cgpu_free(MD->ppMtlQueues[i]);
        }
    }
    return;
}

// API Objects APIs
CGPUFenceId cgpu_create_fence_metal(CGPUDeviceId device)
{
    CGPUFence_Metal* MF = (CGPUFence_Metal*)cgpu_calloc(1, sizeof(CGPUFence_Metal));
    MF->pMtlSemaphore = dispatch_semaphore_create(0);
    MF->mSubmitted = false;
    return &MF->super;
}

RUNTIME_API void cgpu_free_fence_metal(CGPUFenceId fence)
{
    CGPUFence_Metal* MF = (CGPUFence_Metal*)fence;
    MF->pMtlSemaphore = nil;
    cgpu_free(MF);
}

// Queue APIs
CGPUQueueId cgpu_get_queue_metal(CGPUDeviceId device, ECGPUQueueType type, uint32_t index)
{
    CGPUQueue_Metal* MQ = (CGPUQueue_Metal*)cgpu_calloc(1, sizeof(CGPUQueue_Metal));
    CGPUDevice_Metal* MD = (CGPUDevice_Metal*)cgpu_calloc(1, sizeof(CGPUDevice_Metal));
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
    cgpu_assert(0 && "No impl!");
}

void cgpu_wait_queue_idle_metal(CGPUQueueId queue)
{
    CGPUQueue_Metal* MQ = (CGPUQueue_Metal*)queue;
    id<MTLCommandBuffer> waitCmdBuf =
    [MQ->mtlCommandQueue commandBufferWithUnretainedReferences];

    [waitCmdBuf commit];
    [waitCmdBuf waitUntilCompleted];
    waitCmdBuf = nil;
}

void cgpu_free_queue_metal(CGPUQueueId queue)
{
    CGPUQueue_Metal* MQ = (CGPUQueue_Metal*)queue;
    MQ->mtlCommandQueue = nil;
#if defined(ENABLE_FENCES)
    if (@available(macOS 10.13, iOS 10.0, *))
    {
        MQ->mtlQueueFence = nil;
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
    MB->mtlBlitEncoder = nil;
    MB->cmptEncoder.mtlComputeEncoder = nil;
    MB->renderEncoder.mtlRenderEncoder = nil;
    cgpu_free(MB);
}

void cgpu_free_command_pool_metal(CGPUCommandPoolId pool)
{
    CGPUCommandPool_Metal* PQ = (CGPUCommandPool_Metal*)pool;
    cgpu_free(PQ);
}

// Helpers
NSArray<id<MTLDevice>>* MetalUtil_GetAvailableMTLDeviceArray()
{
    NSMutableArray* mtlDevs = [NSMutableArray array];
#ifndef TARGET_IOS
    NSArray* rawMTLDevs = [MTLCopyAllDevices() autorelease];
    if (rawMTLDevs)
    {
        const bool forceLowPower = false;

        // Populate the array of appropriate MTLDevices
        for (id<MTLDevice> md in rawMTLDevs)
        {
            if (!forceLowPower || md.isLowPower) { [mtlDevs addObject:md]; }
        }

        // Sort by power
        [mtlDevs sortUsingComparator:^(id<MTLDevice> md1, id<MTLDevice> md2) {
            BOOL md1IsLP = md1.isLowPower;
            BOOL md2IsLP = md2.isLowPower;

            if (md1IsLP == md2IsLP)
            {
                // If one device is headless and the other one is not, select the
                // one that is not headless first.
                BOOL md1IsHeadless = md1.isHeadless;
                BOOL md2IsHeadless = md2.isHeadless;
                if (md1IsHeadless == md2IsHeadless)
                {
                    return NSOrderedSame;
                }
                return md2IsHeadless ? NSOrderedAscending : NSOrderedDescending;
            }

            return md2IsLP ? NSOrderedAscending : NSOrderedDescending;
        }];
    }
#else  // _IOS_OR_TVOS
    id<MTLDevice> md = [MTLCreateSystemDefaultDevice() autorelease];
    if (md) { [mtlDevs addObject:md]; }
#endif // TARGET_IOS

    return mtlDevs; // retained
}
