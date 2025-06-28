#include "SkrBase/misc/debug.h"
#include "SkrGraphics/raytracing.h"
#include "metal_utils.h"

const CGPURayTracingProcTable rt_tbl_metal = {
    .create_acceleration_structure = &cgpu_create_acceleration_structure_metal,
    .free_acceleration_structure = &cgpu_free_acceleration_structure_metal,
    .cmd_build_acceleration_structure = &cgpu_cmd_build_acceleration_structures_metal,
};

const struct CGPURayTracingProcTable* CGPU_MetalRayTracingProcTable()
{
    return &rt_tbl_metal;
}

inline static MTLAccelerationStructureInstanceOptions ToMTLASOptions(CGPUAccelerationStructureInstanceFlags flags);

CGPUAccelerationStructureId cgpu_create_acceleration_structure_metal(CGPUDeviceId device, const struct CGPUAccelerationStructureDescriptor* desc)
{
    CGPUDevice_Metal* D = (CGPUDevice_Metal*)device;
    CGPUAccelerationStructure_Metal* AS = nil;
    size_t memSize = sizeof(CGPUAccelerationStructure_Metal);
    MTLAccelerationStructureSizes accelSizes = {};
    if (desc->type == CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
    {
        AS = (CGPUAccelerationStructure_Metal*)cgpu_calloc(1, memSize);
        AS->super.type = CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        AS->descriptor_count = desc->bottom.count;
        NSMutableArray<MTLAccelerationStructureTriangleGeometryDescriptor*>* geomDescs = [[NSMutableArray alloc] init];

        // Create a primitive acceleration structure descriptor to contain all the triangle geometry.
        MTLPrimitiveAccelerationStructureDescriptor* asDesc = [MTLPrimitiveAccelerationStructureDescriptor descriptor];
        for (uint32_t j = 0; j < AS->descriptor_count; ++j)
        {
            const CGPUAccelerationStructureGeometryDesc* pGeom = &desc->bottom.geometries[j];
            cgpu_assert(pGeom->vertex_buffer);
            cgpu_assert(pGeom->vertex_count);

            // Fill vertex buffer
            MTLAccelerationStructureTriangleGeometryDescriptor* geomDesc = [MTLAccelerationStructureTriangleGeometryDescriptor descriptor];
            CGPUBuffer_Metal* pVertexBuffer = (CGPUBuffer_Metal*)pGeom->vertex_buffer;
            geomDesc.vertexBuffer = pVertexBuffer->mtlBuffer;
            geomDesc.vertexBufferOffset = pGeom->vertex_offset;
            geomDesc.vertexStride = pGeom->vertex_stride;
            // geomDesc.vertexFormat = ToMTLAttributeFormat(pGeom->mVertexFormat);

            // Fill index buffer
            if (pGeom->index_count)
            {
                cgpu_assert(pGeom->index_buffer);
                CGPUBuffer_Metal* pIndexBuffer = (CGPUBuffer_Metal*)pGeom->index_buffer;
                geomDesc.indexBuffer = pIndexBuffer->mtlBuffer;
                geomDesc.indexBufferOffset = pGeom->index_offset;
                geomDesc.indexType = (pGeom->index_stride == sizeof(uint16_t) ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32);
                geomDesc.triangleCount = pGeom->index_count / 3;
            }
            else
            {
                geomDesc.triangleCount = pGeom->vertex_count / 3;
            }
            [geomDescs addObject:geomDesc];
        }
        asDesc.geometryDescriptors = geomDescs;
        AS->asBottom.mtlBottomDesc = asDesc;
        accelSizes = [D->pDevice accelerationStructureSizesWithDescriptor:asDesc];
    }
    else if (desc->type == CGPU_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL)
    {
        // Pre-collect
        NSMutableArray<id<MTLAccelerationStructure>>* primitiveASArray = [[NSMutableArray alloc] init];
        NSMutableArray<id<MTLAccelerationStructure>>* uniqueBottomAS = [[NSMutableArray alloc] init];
        uint32_t bottomReferenceCount = 0;
        for (uint32_t i = 0; i < desc->top.count; ++i)
        {
            const CGPUAccelerationStructureInstanceDesc* pInst = &desc->top.instances[i];
            const CGPUAccelerationStructure_Metal* pBottomAS = (const CGPUAccelerationStructure_Metal*)pInst->bottom;
            [primitiveASArray addObject:pBottomAS->mtlAS];
        }
        [uniqueBottomAS setArray:[[NSSet setWithArray:primitiveASArray] allObjects]];
        bottomReferenceCount = (uint32_t)[uniqueBottomAS count];
        memSize += bottomReferenceCount * sizeof(id);

        // Setup AS
        AS = (CGPUAccelerationStructure_Metal*)cgpu_calloc(1, memSize);
        AS->super.type = CGPU_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        AS->descriptor_count = desc->top.count;
        AS->asTop.mtlBottomASRefsCount = bottomReferenceCount;

        // Collect refs to BLASes
        void* mem = (AS + 1);
        AS->asTop.mtlBottomASRefs = (__unsafe_unretained id*)(mem);
        for (uint32_t i = 0; i < AS->asTop.mtlBottomASRefsCount; ++i)
        {
            AS->asTop.mtlBottomASRefs[i] = uniqueBottomAS[i];
        }

        MTLInstanceAccelerationStructureDescriptor* asDesc = [MTLInstanceAccelerationStructureDescriptor descriptor];
        CGPUBufferDescriptor instanceDesc = {};
        instanceDesc.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
        instanceDesc.flags = CGPU_BCF_PERSISTENT_MAP_BIT;
        instanceDesc.size = sizeof(MTLAccelerationStructureInstanceDescriptor) * AS->descriptor_count;
        AS->instance_desc_buffer = cgpu_create_buffer(device, &instanceDesc);

        MTLAccelerationStructureInstanceDescriptor* instanceDescs =
            (MTLAccelerationStructureInstanceDescriptor*)AS->instance_desc_buffer->info->cpu_mapped_address;

        for (uint32_t i = 0; i < desc->top.count; ++i)
        {
            const CGPUAccelerationStructureInstanceDesc* pInst = &desc->top.instances[i];
            const CGPUAccelerationStructure_Metal* pBLAS = (const CGPUAccelerationStructure_Metal*)pInst->bottom;
            MTLAccelerationStructureInstanceDescriptor* ToBuild = &instanceDescs[pInst->instance_id];

            cgpu_assert(pInst->bottom);
            ToBuild->options = ToMTLASOptions(pInst->flags);
            ToBuild->mask = pInst->instance_mask;
            ToBuild->accelerationStructureIndex = (uint32_t)[primitiveASArray indexOfObject:pBLAS->mtlAS];
            // Copy the first three rows of the instance transformation matrix. Metal
            // assumes that the bottom row is (0, 0, 0, 1), which allows the renderer to
            // tightly pack instance descriptors in memory.
            for (uint32_t column = 0; column < 4; ++column)
            {
                for (uint32_t row = 0; row < 3; ++row)
                {
                    ToBuild->transformationMatrix.columns[column].elements[row] = pInst->transform[row * 4 + column];
                }
            }
        }

        const CGPUBuffer_Metal* pInstanceDescBuffer = (CGPUBuffer_Metal*)AS->instance_desc_buffer;
        asDesc.instanceCount = AS->descriptor_count;
        asDesc.instanceDescriptorBuffer = pInstanceDescBuffer->mtlBuffer;
        asDesc.instancedAccelerationStructures = primitiveASArray;

        AS->asTop.mtlTopDesc = asDesc;
        accelSizes = [D->pDevice accelerationStructureSizesWithDescriptor:asDesc];
    }
    AS->super.scratch_buffer_size = accelSizes.buildScratchBufferSize;
    AS->mtlAS = [D->pDevice newAccelerationStructureWithSize:accelSizes.accelerationStructureSize];
    cgpu_assert(AS->mtlAS);

    CGPUBufferDescriptor scratchBufferDesc = {};
    scratchBufferDesc.descriptors = CGPU_RESOURCE_TYPE_RW_BUFFER;
    scratchBufferDesc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
    scratchBufferDesc.flags = CGPU_BCF_NO_DESCRIPTOR_VIEW_CREATION;
    scratchBufferDesc.size = accelSizes.buildScratchBufferSize;
    AS->scratch_buffer = cgpu_create_buffer(device, &scratchBufferDesc);
    return &AS->super;
}

void cgpu_free_acceleration_structure_metal(CGPUAccelerationStructureId as)
{
    CGPUAccelerationStructure_Metal* AS = (CGPUAccelerationStructure_Metal*)as;
    if (AS->super.type == CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL) 
    {
        AS->asBottom.mtlBottomDesc = nil;
    } 
    else if (AS->super.type == CGPU_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL) 
    {
        AS->asTop.mtlTopDesc = nil;
    }
    AS->mtlAS = nil;

    if (AS->instance_desc_buffer)
        cgpu_free_buffer(AS->instance_desc_buffer);
    if (AS->scratch_buffer)
        cgpu_free_buffer(AS->scratch_buffer);
    
    cgpu_free(AS);
}

void cgpu_cmd_build_acceleration_structures_metal(CGPUCommandBufferId cmd, const struct CGPUAccelerationStructureBuildDescriptor* desc)
{
    CGPUCommandBuffer_Metal* CMD = (CGPUCommandBuffer_Metal*)cmd;
    MetalUtil_FlushUtilEncoders(CMD, MTLUtilEncoderTypeBlit);

    CMD->UtilEncoders.mtlASCommandEncoder = CMD->UtilEncoders.mtlASCommandEncoder ? CMD->UtilEncoders.mtlASCommandEncoder : [CMD->mtlCommandBuffer accelerationStructureCommandEncoder];
    for (uint32_t i = 0; i < desc->as_count; i++)
    {
        const CGPUAccelerationStructure_Metal* AS = (const CGPUAccelerationStructure_Metal*)desc->as[i];
        const CGPUBuffer_Metal* pScratchBuffer = (const CGPUBuffer_Metal*)AS->scratch_buffer;
        if (desc->type == CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
        {
            [CMD->UtilEncoders.mtlASCommandEncoder buildAccelerationStructure:AS->mtlAS
                                            descriptor:AS->asBottom.mtlBottomDesc
                                        scratchBuffer:pScratchBuffer->mtlBuffer
                                    scratchBufferOffset:0];
        }
        else
        {
            [CMD->UtilEncoders.mtlASCommandEncoder buildAccelerationStructure:AS->mtlAS
                                            descriptor:AS->asTop.mtlTopDesc
                                        scratchBuffer:pScratchBuffer->mtlBuffer
                                    scratchBufferOffset:0];
        }
    }

}

inline static MTLAccelerationStructureInstanceOptions ToMTLASOptions(CGPUAccelerationStructureInstanceFlags flags)
{
    MTLAccelerationStructureInstanceOptions ret = MTLAccelerationStructureInstanceOptionNone;
    if (flags & CGPU_ACCELERATION_STRUCTURE_INSTANCE_FLAG_FORCE_OPAQUE)
        ret |= MTLAccelerationStructureInstanceOptionOpaque;
    if (flags & CGPU_ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE)
        ret |= MTLAccelerationStructureInstanceOptionDisableTriangleCulling;
    if (flags & CGPU_ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE)
        ret |= MTLAccelerationStructureInstanceOptionTriangleFrontFacingWindingCounterClockwise;
    return ret;
}