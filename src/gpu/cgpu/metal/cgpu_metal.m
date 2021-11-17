#include "cgpu/backend/metal/cgpu_metal.h"
#include "cgpu/backend/metal/cgpu_metal_types.h"

const CGpuProcTable tbl_metal = 
{
    .create_instance = &cgpu_create_instance_metal,
    .query_instance_features = &cgpu_query_instance_features_metal,
    .free_instance = &cgpu_free_instance_metal,

    .enum_adapters = &cgpu_enum_adapters_metal,
    .query_adapter_detail = &cgpu_query_adapter_detail_metal,
    .query_queue_count = &cgpu_query_queue_count_metal
};

const CGpuProcTable* CGPU_MetalProcTable()
{
    return &tbl_metal;
}

// Instance APIs
CGpuInstanceId cgpu_create_instance_metal(CGpuInstanceDescriptor const* descriptor)
{
	CGpuInstance_Metal* MI = (CGpuInstance_Metal*)cgpu_calloc(1, sizeof(CGpuInstance_Metal));
    MI->adapter.pDevice = MTLCreateSystemDefaultDevice();
    // Query Adapter Informations

    return &MI->super;
}

void cgpu_query_instance_features_metal(CGpuInstanceId instance, struct CGpuInstanceFeatures* features)
{
	features->specialization_constant = true;
}

void cgpu_free_instance_metal(CGpuInstanceId instance)
{
	CGpuInstance_Metal* MI = (CGpuInstance_Metal*)instance;
    MI->adapter.pDevice = nil;
    cgpu_free(MI);
}

// Adapter APIs
void cgpu_enum_adapters_metal(CGpuInstanceId instance, CGpuAdapterId* const adapters, uint32_t* adapters_num)
{
    CGpuInstance_Metal* MI = (CGpuInstance_Metal*)instance;
    
    *adapters_num = 1;
    if (adapters != CGPU_NULLPTR)
	{
		adapters[0] = &MI->adapter.super;
	}
}

void cgpu_query_adapter_detail_metal(const CGpuAdapterId adapter, struct CGpuAdapterDetail* detail)
{

}

uint32_t cgpu_query_queue_count_metal(const CGpuAdapterId adapter, const ECGpuQueueType type)
{
    return 0;
}
