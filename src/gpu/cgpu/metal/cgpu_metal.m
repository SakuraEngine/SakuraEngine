#include "cgpu/backend/metal/cgpu_metal.h"

const CGpuProcTable tbl_metal = 
{
    .create_instance = &cgpu_create_instance_metal,
    .query_instance_features = &cgpu_query_instance_features_metal,
    .free_instance = &cgpu_free_instance_metal
};

const CGpuProcTable* CGPU_MetalProcTable()
{
    return &tbl_metal;
}

CGpuInstanceId cgpu_create_instance_metal(CGpuInstanceDescriptor const* descriptor)
{
	CGpuInstance_Metal* I = (CGpuInstance_Metal*)cgpu_calloc(1, sizeof(CGpuInstance_Metal));

    return &I->super;
}

void cgpu_query_instance_features_metal(CGpuInstanceId instance, struct CGpuInstanceFeatures* features)
{
	features->specialization_constant = true;
}

void cgpu_free_instance_metal(CGpuInstanceId instance)
{
	CGpuInstance_Metal* I = (CGpuInstance_Metal*)instance;

    cgpu_free(I);
}