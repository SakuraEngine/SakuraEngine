#include "metal_utils.h"
#include "cgpu/backend/metal/cgpu_metal.h"
#include "cgpu/backend/metal/cgpu_metal_types.h"

const CGpuProcTable tbl_metal = {
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

const CGpuProcTable* CGPU_MetalProcTable()
{
    return &tbl_metal;
}

NSArray<id<MTLDevice>>* MetalUtil_GetAvailableMTLDeviceArray();

// Instance APIs
CGpuInstanceId cgpu_create_instance_metal(CGpuInstanceDescriptor const* descriptor)
{
    CGpuInstance_Metal* MI = (CGpuInstance_Metal*)cgpu_calloc(1, sizeof(CGpuInstance_Metal));
    @autoreleasepool
    {
        NSArray<id<MTLDevice>>* mtlDevices = MetalUtil_GetAvailableMTLDeviceArray();
        MI->adapters = cgpu_calloc(mtlDevices.count, sizeof(CGpuAdapter_Metal));
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

void cgpu_query_instance_features_metal(CGpuInstanceId instance, struct CGpuInstanceFeatures* features)
{
    features->specialization_constant = true;
}

void cgpu_free_instance_metal(CGpuInstanceId instance)
{
    CGpuInstance_Metal* MI = (CGpuInstance_Metal*)instance;
    for (uint32_t i = 0; i < MI->adapters_count; i++)
    {
        MI->adapters[i].device.pDevice = nil;
    }
    cgpu_free(MI->adapters);
    cgpu_free(MI);
}

// Adapter APIs
void cgpu_enum_adapters_metal(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num)
{
    CGpuInstance_Metal* MI = (CGpuInstance_Metal*)instance;

    *adapters_num = MI->adapters_count;
    if (adapters != CGPU_NULLPTR)
    {
        for (uint32_t i = 0; i < MI->adapters_count; i++)
        {
            adapters[i] = &MI->adapters[i].super;
        }
    }
}

const CGpuAdapterDetail* cgpu_query_adapter_detail_metal(const CGpuAdapterId adapter)
{
    CGpuAdapter_Metal* MA = (CGpuAdapter_Metal*)adapter;
    return &MA->adapter_detail;
}

uint32_t cgpu_query_queue_count_metal(const CGpuAdapterId adapter, const ECGpuQueueType type)
{
    return UINT32_MAX;
}

// Device APIs
CGpuDeviceId cgpu_create_device_metal(CGpuAdapterId adapter, const CGpuDeviceDescriptor* desc)
{
    CGpuAdapter_Metal* MA = (CGpuAdapter_Metal*)adapter;
    // Create Requested Queues
    for (uint32_t i = 0; i < desc->queueGroupCount; i++)
    {
        const CGpuQueueGroupDescriptor* queueGroup = desc->queueGroups + i;
        const ECGpuQueueType type = queueGroup->queueType;
        MA->device.ppMtlQueues[type] = cgpu_calloc(queueGroup->queueCount, sizeof(id<MTLCommandQueue>));
        MA->device.pMtlQueueCounts[type] = queueGroup->queueCount;
        for (uint32_t j = 0u; j < queueGroup->queueCount; j++)
        {
            MA->device.ppMtlQueues[type][j] = [MA->device.pDevice newCommandQueueWithMaxCommandBufferCount:512];
        }
    }
    return &MA->device.super;
}

void cgpu_free_device_metal(CGpuDeviceId device)
{
    CGpuDevice_Metal* MD = (CGpuDevice_Metal*)device;
    for (uint32_t i = 0; i < ECGpuQueueType_Count; i++)
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
CGpuFenceId cgpu_create_fence_metal(CGpuDeviceId device)
{
    CGpuFence_Metal* MF = (CGpuFence_Metal*)cgpu_calloc(1, sizeof(CGpuFence_Metal));
    MF->pMtlSemaphore = dispatch_semaphore_create(0);
    MF->mSubmitted = false;
    return &MF->super;
}

RUNTIME_API void cgpu_free_fence_metal(CGpuFenceId fence)
{
    CGpuFence_Metal* MF = (CGpuFence_Metal*)fence;
    MF->pMtlSemaphore = nil;
    cgpu_free(MF);
}

// Queue APIs
CGpuQueueId cgpu_get_queue_metal(CGpuDeviceId device, ECGpuQueueType type, uint32_t index)
{
    CGpuQueue_Metal* MQ = (CGpuQueue_Metal*)cgpu_calloc(1, sizeof(CGpuQueue_Metal));
    CGpuDevice_Metal* MD = (CGpuDevice_Metal*)cgpu_calloc(1, sizeof(CGpuDevice_Metal));
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

void cgpu_submit_queue_metal(CGpuQueueId queue, const struct CGpuQueueSubmitDescriptor* desc)
{
    assert(0 && "No impl!");
}

void cgpu_wait_queue_idle_metal(CGpuQueueId queue)
{
    CGpuQueue_Metal* MQ = (CGpuQueue_Metal*)queue;
    id<MTLCommandBuffer> waitCmdBuf =
        [MQ->mtlCommandQueue commandBufferWithUnretainedReferences];

    [waitCmdBuf commit];
    [waitCmdBuf waitUntilCompleted];
    waitCmdBuf = nil;
}

void cgpu_free_queue_metal(CGpuQueueId queue)
{
    CGpuQueue_Metal* MQ = (CGpuQueue_Metal*)queue;
    MQ->mtlCommandQueue = nil;
#if defined(ENABLE_FENCES)
    if (@available(macOS 10.13, iOS 10.0, *))
    {
        MQ->mtlQueueFence = nil;
    }
#endif
}

// Command APIs
CGpuCommandPoolId cgpu_create_command_pool_metal(CGpuQueueId queue, const CGpuCommandPoolDescriptor* desc)
{
    CGpuCommandPool_Metal* PQ = (CGpuCommandPool_Metal*)cgpu_calloc(1, sizeof(CGpuCommandPool_Metal));
    return &PQ->super;
}

CGpuCommandBufferId cgpu_create_command_buffer_metal(CGpuCommandPoolId pool, const struct CGpuCommandBufferDescriptor* desc)
{
    CGpuCommandBuffer_Metal* MB = (CGpuCommandBuffer_Metal*)cgpu_calloc(1, sizeof(CGpuCommandBuffer_Metal));
    CGpuQueue_Metal* MQ = (CGpuQueue_Metal*)pool->queue;
    MB->mtlCommandBuffer = [MQ->mtlCommandQueue commandBuffer];
    return &MB->super;
}

void cgpu_free_command_buffer_metal(CGpuCommandBufferId cmd)
{
    CGpuCommandBuffer_Metal* MB = (CGpuCommandBuffer_Metal*)cmd;
    MB->mtlCommandBuffer = nil;
    MB->mtlBlitEncoder = nil;
    MB->cmptEncoder.mtlComputeEncoder = nil;
    MB->renderEncoder.mtlRenderEncoder = nil;
    cgpu_free(MB);
}

void cgpu_free_command_pool_metal(CGpuCommandPoolId pool)
{
    CGpuCommandPool_Metal* PQ = (CGpuCommandPool_Metal*)pool;
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
