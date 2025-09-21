#pragma once
#include "SkrGraphics/backend/d3d12/cgpu_d3d12.h"

#ifdef __cplusplus
CGPU_EXTERN_C_BEGIN
#endif

CGPU_API const CGPURayTracingProcTable* CGPU_D3D12RayTracingProcTable();

CGPU_API CGPUAccelerationStructureId cgpu_create_acceleration_structure_d3d12(CGPUDeviceId device, const struct CGPUAccelerationStructureDescriptor* desc);
CGPU_API void cgpu_free_acceleration_structure_d3d12(CGPUAccelerationStructureId as);
CGPU_API void cgpu_cmd_build_acceleration_structures_d3d12(CGPUCommandBufferId cmd, const struct CGPUAccelerationStructureBuildDescriptor* desc);

CGPU_API CGPURayPipelineId cgpu_create_ray_pipeline_d3d12(CGPUDeviceId device, const struct CGPURayPipelineDescriptor* desc);
CGPU_API void cgpu_compute_encoder_bind_ray_pipeline_d3d12(CGPUComputePassEncoderId encoder, CGPURayPipelineId pipeline);
CGPU_API void cgpu_compute_encoder_dispatch_rays_d3d12(CGPUComputePassEncoderId encoder, const struct CGPUDispatchRaysDescriptor* desc);
CGPU_API void cgpu_free_ray_pipeline_d3d12(CGPURayPipelineId pipeline);

typedef struct CGPUAccelerationStructure_D3D12 {
    CGPUAccelerationStructure super;
    CGPUBufferId pASBuffer;
    CGPUBufferViewId pASBufferView;
    CGPUBufferId pScratchBuffer;
    union
    {
        struct
        {
            CGPUBufferId pTransformBuffer;
            D3D12_RAYTRACING_GEOMETRY_DESC* pGeometryDescs;
        } asBottom;
        struct
        {
            CGPUBufferId pInstanceDescBuffer;
            CGPUAccelerationStructure_D3D12** ppBLASRefs;
            uint32_t mBLASRefCount;
        } asTop;
    };
    bool bIsDirty;
    uint32_t mDescCount;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS mFlags;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE mType;
} CGPUAccelerationStructure_D3D12;

typedef struct CGPURayPipelineBase_D3D12 {
    CGPURayPipeline super;
    ID3D12StateObject* StateObject;
    ID3D12StateObjectProperties* StateObjectProps;
    CGPUBufferId SBT;

    D3D12_GPU_VIRTUAL_ADDRESS_RANGE RayGenerationShaderRecord;
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE MissShaderTable;
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE HitGroupTable;
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE CallableShaderTable;
} CGPURayPipelineBase_D3D12;

#ifdef __cplusplus
CGPU_EXTERN_C_END
#endif