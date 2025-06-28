#pragma once
#include "SkrGraphics/api.h"

DEFINE_CGPU_OBJECT(CGPUAccelerationStructure);
struct CGPUAccelerationStructureBuildDescriptor;

typedef enum ECGPUAccelerationStructureType {
    CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL = 0,
    CGPU_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL = 1,
} ECGPUAccelerationStructureType;

typedef enum ECGPUAccelerationStructureBuildFlagBits {
    CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE = 0x0,
    CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE = 0x1,
    CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION = 0x2,
    CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE = 0x4,
    CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD = 0x8,
    CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY = 0x10,
    CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE = 0x20,
} ECGPUAccelerationStructureBuildFlagBits;
typedef uint32_t CGPUAccelerationStructureBuildFlags;

typedef enum ECGPUAccelerationStructureGeometryFlagBits
{
    CGPU_ACCELERATION_STRUCTURE_GEOMETRY_FLAG_NONE = 0,
    CGPU_ACCELERATION_STRUCTURE_GEOMETRY_FLAG_OPAQUE = 0x1,
    CGPU_ACCELERATION_STRUCTURE_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION = 0x2
} ECGPUAccelerationStructureGeometryFlagBits;
typedef uint32_t CGPUAccelerationStructureGeometryFlags;

// Rustam: check if this can be mapped to Metal
typedef enum ECGPUAccelerationStructureInstanceFlagBits
{
    CGPU_ACCELERATION_STRUCTURE_INSTANCE_FLAG_NONE = 0,
    CGPU_ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE = 0x1,
    CGPU_ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE = 0x2,
    CGPU_ACCELERATION_STRUCTURE_INSTANCE_FLAG_FORCE_OPAQUE = 0x4,
    CGPU_ACCELERATION_STRUCTURE_INSTANCE_FLAG_FORCE_NON_OPAQUE = 0x8
} ECGPUAccelerationStructureInstanceFlagBits;
typedef uint32_t CGPUAccelerationStructureInstanceFlags;

CGPU_EXTERN_C CGPU_API CGPUAccelerationStructureId cgpu_create_acceleration_structure(CGPUDeviceId device, const struct CGPUAccelerationStructureDescriptor* desc);
typedef CGPUAccelerationStructureId (*CGPUProcCreateAccelerationStructure)(CGPUDeviceId device, const struct CGPUAccelerationStructureDescriptor* desc);

CGPU_EXTERN_C CGPU_API void cgpu_free_acceleration_structure(CGPUAccelerationStructureId as);
typedef void (*CGPUProcFreeAccelerationStructure)(CGPUAccelerationStructureId as);

CGPU_EXTERN_C CGPU_API void cgpu_cmd_build_acceleration_structures(CGPUCommandBufferId cmd, const struct CGPUAccelerationStructureBuildDescriptor* desc);
typedef void (*CGPUProcCmdBuildAccelerationStructure)(CGPUCommandBufferId cmd, const struct CGPUAccelerationStructureBuildDescriptor* desc);

// TODO: MANUAL SCRATCH BUFFER MANAGEMENT
// CGPU_EXTERN_C CGPU_API CGPUBufferId cgpu_create_tlas_scratch_buffer(CGPUDeviceId device, const CGPUAccelerationStructureId* as, uint32_t as_count);
// CGPU_EXTERN_C CGPU_API CGPUBufferId cgpu_create_tlas_scratch_buffer_with_size(CGPUDeviceId device, uint32_t size);

typedef struct CGPURayTracingProcTable {
    CGPUProcCreateAccelerationStructure create_acceleration_structure;
    CGPUProcFreeAccelerationStructure free_acceleration_structure;
    CGPUProcCmdBuildAccelerationStructure cmd_build_acceleration_structure;
} CGPURayTracingProcTable;

typedef struct CGPUAccelerationStructureGeometryDesc
{
    CGPUBufferId vertex_buffer;
    CGPUBufferId index_buffer;
    uint32_t vertex_offset;
    uint32_t vertex_count;
    uint32_t vertex_stride;
    ECGPUFormat vertex_format;
    uint32_t index_offset;
    uint32_t index_count;
    uint32_t index_stride;
    CGPUAccelerationStructureGeometryFlags flags;
} CGPUAccelerationStructureGeometryDesc;

typedef struct CGPUAccelerationStructureInstanceDesc
{
    CGPUAccelerationStructureId bottom;
    float transform[12];
    CGPUAccelerationStructureInstanceFlags flags;
    uint32_t instance_id;
    uint32_t instance_mask;
    // uint32_t mInstanceContributionToHitGroupIndex;
} CGPUAccelerationStructureInstanceDesc;

typedef struct CGPUAccelerationStructureDescriptor {
    ECGPUAccelerationStructureType type;
    CGPUAccelerationStructureBuildFlags flags;
    union {
        struct {
            uint32_t count;
            const CGPUAccelerationStructureGeometryDesc* geometries;
        } bottom;
        struct {
            uint32_t count;
            const CGPUAccelerationStructureInstanceDesc* instances;
        } top;
    };
} CGPUAccelerationStructureDescriptor;

typedef struct CGPUAccelerationStructureBuildDescriptor {
    ECGPUAccelerationStructureType type;
    uint32_t as_count;
    const CGPUAccelerationStructureId* as;
} CGPUAccelerationStructureBuildDescriptor;

typedef struct CGPUAccelerationStructure {
    CGPUDeviceId device;
    ECGPUAccelerationStructureType type;
    uint32_t scratch_buffer_size;
} CGPUAccelerationStructure;