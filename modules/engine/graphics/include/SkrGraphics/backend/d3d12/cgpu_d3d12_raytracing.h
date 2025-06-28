#pragma once
#include "SkrGraphics/backend/d3d12/cgpu_d3d12.h"

#ifdef __cplusplus
CGPU_EXTERN_C_BEGIN
#endif

CGPU_API const CGPURayTracingProcTable* CGPU_D3D12RayTracingProcTable();

CGPU_API CGPUAccelerationStructureId cgpu_create_acceleration_structure_d3d12(CGPUDeviceId device, const struct CGPUAccelerationStructureDescriptor* desc);
CGPU_API void cgpu_free_acceleration_structure_d3d12(CGPUAccelerationStructureId as);
CGPU_API void cgpu_cmd_build_acceleration_structures_d3d12(CGPUCommandBufferId cmd, const struct CGPUAccelerationStructureBuildDescriptor* desc);

typedef struct CGPUAccelerationStructure_D3D12 {
    CGPUAccelerationStructure super;
    CGPUBufferId pASBuffer;
    CGPUBufferId pScratchBuffer;
    union
    {
        struct
        {
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

#ifdef __cplusplus
CGPU_EXTERN_C_END
#endif