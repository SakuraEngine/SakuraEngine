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
    .free_device = &cgpu_free_device_metal

    // Queue APIs
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
