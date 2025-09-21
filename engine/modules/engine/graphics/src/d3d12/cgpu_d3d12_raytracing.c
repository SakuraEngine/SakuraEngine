#include "SkrGraphics/backend/d3d12/cgpu_d3d12_raytracing.h"
#include "d3d12_utils.h"

const CGPURayTracingProcTable rt_tbl_d3d12 = {
    .create_acceleration_structure = &cgpu_create_acceleration_structure_d3d12,
    .free_acceleration_structure = &cgpu_free_acceleration_structure_d3d12,
    .cmd_build_acceleration_structure = &cgpu_cmd_build_acceleration_structures_d3d12,
};

const CGPURayTracingProcTable* CGPU_D3D12RayTracingProcTable()
{
    return &rt_tbl_d3d12;
}

inline static D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE ToDXRASType(ECGPUAccelerationStructureType type);
inline static D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS ToDXRBuildFlags(CGPUAccelerationStructureBuildFlags flags);
inline static D3D12_RAYTRACING_GEOMETRY_FLAGS ToDXRGeometryFlags(CGPUAccelerationStructureGeometryFlags flags);
inline static D3D12_RAYTRACING_INSTANCE_FLAGS ToDXRInstanceFlags(CGPUAccelerationStructureInstanceFlags flags);

CGPUAccelerationStructureId cgpu_create_acceleration_structure_d3d12(CGPUDeviceId device, const struct CGPUAccelerationStructureDescriptor* desc)
{
    CGPUDevice_D3D12* D = (CGPUDevice_D3D12*)device;
    const CGPUAdapter_D3D12* A = (CGPUAdapter_D3D12*)device->adapter;
    size_t memSize = sizeof(CGPUAccelerationStructure_D3D12);
    if (desc->type == CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
    {
        memSize += desc->bottom.count * sizeof(D3D12_RAYTRACING_GEOMETRY_DESC);
    }

    CGPUAccelerationStructure_D3D12* AS = (CGPUAccelerationStructure_D3D12*)cgpu_calloc(1, memSize);
    AS->mType = ToDXRASType(desc->type);
    AS->mFlags = ToDXRBuildFlags(desc->flags);
    AS->super.type = desc->type;

    AS->super.scratch_buffer_size = 0;
    if (desc->type == CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
    {
        SkrCZoneN(zz, "CreateBLAS", 1);
        AS->mDescCount = desc->bottom.count;
        AS->asBottom.pGeometryDescs = (D3D12_RAYTRACING_GEOMETRY_DESC*)(AS + 1);
        CGPUBufferDescriptor tramsformBufferDesc = {
            .memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU,
            .flags = CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT,
            .size = desc->bottom.count * sizeof(float) * 4 * 3,
            .name = "BLAS-TransformBuffer"
        };
        AS->asBottom.pTransformBuffer = cgpu_create_buffer(device, &tramsformBufferDesc);
        const CGPUBuffer_D3D12* pTransformBuffer = (const CGPUBuffer_D3D12*)AS->asBottom.pTransformBuffer;
        for (uint32_t j = 0; j < AS->mDescCount; ++j)
        {
            const CGPUAccelerationStructureGeometryDesc* pGeom = &desc->bottom.geometries[j];
            D3D12_RAYTRACING_GEOMETRY_DESC* pGeomD3D12 = &AS->asBottom.pGeometryDescs[j];

            pGeomD3D12->Flags = ToDXRGeometryFlags(pGeom->flags);
            memcpy((uint8_t*)AS->asBottom.pTransformBuffer->info->cpu_mapped_address + j * sizeof(float) * 3 * 4, 
                pGeom->transform, sizeof(float) * 3 * 4);

            pGeomD3D12->Triangles.Transform3x4 = pTransformBuffer->mDxGpuAddress + j * sizeof(float) * 3 * 4; 

            if (pGeom->index_count)
            {
                cgpu_assert(pGeom->index_buffer);
                const CGPUBuffer_D3D12* pIndexBuffer = (CGPUBuffer_D3D12*)pGeom->index_buffer;
                pGeomD3D12->Triangles.IndexBuffer = pIndexBuffer->mDxGpuAddress + pGeom->index_offset;
                pGeomD3D12->Triangles.IndexCount = pGeom->index_count;
                pGeomD3D12->Triangles.IndexFormat = (pGeom->index_stride == sizeof(uint16_t) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
            }

            cgpu_assert(pGeom->vertex_buffer);
            cgpu_assert(pGeom->vertex_count);
            const CGPUBuffer_D3D12* pVertexBuffer = (CGPUBuffer_D3D12*)pGeom->vertex_buffer;
            pGeomD3D12->Triangles.VertexBuffer.StartAddress = pVertexBuffer->mDxGpuAddress + pGeom->vertex_offset;
            pGeomD3D12->Triangles.VertexBuffer.StrideInBytes = pGeom->vertex_stride;
            pGeomD3D12->Triangles.VertexCount = pGeom->vertex_count;
            pGeomD3D12->Triangles.VertexFormat = DXGIUtil_TranslatePixelFormat(pGeom->vertex_format, false);
            /*
            Format of the vertices in VertexBuffer. Must be one of the following:

            DXGI_FORMAT_R32G32_FLOAT - third component is assumed 0
            DXGI_FORMAT_R32G32B32_FLOAT
            DXGI_FORMAT_R16G16_FLOAT - third component is assumed 0
            DXGI_FORMAT_R16G16B16A16_FLOAT - A16 component is ignored, other data can be packed there, such as setting vertex stride to 6
            bytes. DXGI_FORMAT_R16G16_SNORM - third component is assumed 0 DXGI_FORMAT_R16G16B16A16_SNORM - A16 component is ignored, other
            data can be packed there, such as setting vertex stride to 6 bytes. Tier 1.1 devices support the following additional formats:

            DXGI_FORMAT_R16G16B16A16_UNORM - A16 component is ignored, other data can be packed there, such as setting vertex stride to 6
            bytes DXGI_FORMAT_R16G16_UNORM - third component assumed 0 DXGI_FORMAT_R10G10B10A2_UNORM - A2 component is ignored, stride must
            be 4 bytes DXGI_FORMAT_R8G8B8A8_UNORM - A8 component is ignored, other data can be packed there, such as setting vertex stride
            to 3 bytes DXGI_FORMAT_R8G8_UNORM - third component assumed 0 DXGI_FORMAT_R8G8B8A8_SNORM - A8 component is ignored, other data
            can be packed there, such as setting vertex stride to 3 bytes DXGI_FORMAT_R8G8_SNORM - third component assumed 0
            */
            cgpu_assert(DXGI_FORMAT_R32G32_FLOAT == pGeomD3D12->Triangles.VertexFormat ||
                DXGI_FORMAT_R32G32B32_FLOAT == pGeomD3D12->Triangles.VertexFormat ||
                DXGI_FORMAT_R16G16_FLOAT == pGeomD3D12->Triangles.VertexFormat ||
                DXGI_FORMAT_R16G16B16A16_FLOAT == pGeomD3D12->Triangles.VertexFormat ||
                DXGI_FORMAT_R16G16_SNORM == pGeomD3D12->Triangles.VertexFormat ||
                DXGI_FORMAT_R16G16B16A16_SNORM == pGeomD3D12->Triangles.VertexFormat ||
                ((A->mRayTracingTier > D3D12_RAYTRACING_TIER_1_0) ? (DXGI_FORMAT_R16G16B16A16_UNORM == pGeomD3D12->Triangles.VertexFormat ||
                                                                        DXGI_FORMAT_R16G16_UNORM == pGeomD3D12->Triangles.VertexFormat ||
                                                                        DXGI_FORMAT_R10G10B10A2_UNORM == pGeomD3D12->Triangles.VertexFormat ||
                                                                        DXGI_FORMAT_R8G8B8A8_UNORM == pGeomD3D12->Triangles.VertexFormat ||
                                                                        DXGI_FORMAT_R8G8_UNORM == pGeomD3D12->Triangles.VertexFormat ||
                                                                        DXGI_FORMAT_R8G8B8A8_SNORM == pGeomD3D12->Triangles.VertexFormat ||
                                                                        DXGI_FORMAT_R8G8_SNORM == pGeomD3D12->Triangles.VertexFormat) :
                                                                    false));
        }
        /************************************************************************/
        // Get the size requirement for the Acceleration Structures
        /************************************************************************/
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc = {
            .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
            .Flags = AS->mFlags,
            .NumDescs = AS->mDescCount,
            .pGeometryDescs = AS->asBottom.pGeometryDescs,
            .Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
        };
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = { 0 };
        {
            SkrCZoneN(zzz, "GetRaytracingAccelerationStructurePrebuildInfo", 1);
            COM_CALL(GetRaytracingAccelerationStructurePrebuildInfo, D->pDxDevice5, &prebuildDesc, &info);
            SkrCZoneEnd(zzz);
        }

        /************************************************************************/
        // Allocate Acceleration Structure Buffer
        /************************************************************************/
        {
            SkrCZoneN(zzz, "CreateASBuffer", 1);
            CGPUBufferDescriptor bufferDesc = {
                .usages = CGPU_BUFFER_USAGE_SHADER_READWRITE,
                .memory_usage = CGPU_MEM_USAGE_GPU_ONLY,
                .flags = CGPU_BUFFER_FLAG_DEDICATED_BIT,
                .size = info.ResultDataMaxSizeInBytes,
                .start_state = CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE,
                .name = "BLAS-Buffer"
            };
            AS->pASBuffer = cgpu_create_buffer(device, &bufferDesc);
            AS->super.scratch_buffer_size = (uint32_t)info.ScratchDataSizeInBytes;
            SkrCZoneEnd(zzz);
        }
        SkrCZoneEnd(zz);
    }
    else if (desc->type == CGPU_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL)
    {
        SkrCZoneN(zz, "CreateTLAS", 1);
        AS->mDescCount = desc->top.count;
        /************************************************************************/
        // Get the size requirement for the Acceleration Structures
        /************************************************************************/
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc = {
            .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
            .Flags = AS->mFlags,
            .NumDescs = desc->top.count,
            .pGeometryDescs = NULL,
            .Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
        };

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = { 0 };
        COM_CALL(GetRaytracingAccelerationStructurePrebuildInfo, D->pDxDevice5, &prebuildDesc, &info);

        /************************************************************************/
        /*  Construct buffer with instances descriptions                        */
        /************************************************************************/
        D3D12_RAYTRACING_INSTANCE_DESC* instanceDescs = cgpu_malloc(desc->top.count * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
        for (uint32_t i = 0; i < desc->top.count; ++i)
        {
            const CGPUAccelerationStructureInstanceDesc* pInst = &desc->top.instances[i];
            CGPUAccelerationStructure_D3D12* pBLAS = (CGPUAccelerationStructure_D3D12*)pInst->bottom;
            cgpu_assert(pBLAS && "cgpu_assert: Instance Bottom AS must not be NULL!");

            const CGPUBuffer_D3D12* pASBuffer = (const CGPUBuffer_D3D12*)pBLAS->pASBuffer;
            instanceDescs[i] = (D3D12_RAYTRACING_INSTANCE_DESC){
                .AccelerationStructure = COM_CALL(GetGPUVirtualAddress, pASBuffer->pDxResource),
                .Flags = ToDXRInstanceFlags(pInst->flags),
                .InstanceContributionToHitGroupIndex = 0, // TODO: support hit group index
                .InstanceID = pInst->instance_id,
                .InstanceMask = pInst->instance_mask,
            };
            memcpy(instanceDescs[i].Transform, pInst->transform, sizeof(float[12]));
        }

        CGPUBufferDescriptor instanceDesc = {
            .memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU,
            .flags = CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT,
            .size = desc->top.count * sizeof(instanceDescs[0]),
            .name = "TLAS-InstancesBuffer"
        };
        AS->asTop.pInstanceDescBuffer = cgpu_create_buffer(device, &instanceDesc);
        if (desc->top.count)
        {
            memcpy(AS->asTop.pInstanceDescBuffer->info->cpu_mapped_address, instanceDescs, instanceDesc.size);
        }
        cgpu_free(instanceDescs);
        /************************************************************************/
        // Allocate Acceleration Structure Buffer
        /************************************************************************/
        CGPUBufferDescriptor bufferDesc = {
            .usages = CGPU_BUFFER_USAGE_SHADER_READWRITE | CGPU_BUFFER_USAGE_SHADER_READ,
            .memory_usage = CGPU_MEM_USAGE_GPU_ONLY,
            .flags = CGPU_BUFFER_FLAG_DEDICATED_BIT,
            .size = info.ResultDataMaxSizeInBytes,
            .start_state = CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE,
            .name = "TLAS-Buffer"
        };
        AS->pASBuffer = cgpu_create_buffer(device, &bufferDesc);

        CGPUBufferViewDescriptor as_desc = {
            .buffer = AS->pASBuffer,
            .view_usages = CGPU_BUFFER_VIEW_USAGE_SRV_RAW,
            .offset = 0,
            .size = 0
        };
        AS->pASBufferView = cgpu_create_buffer_view(device, &as_desc);

        const CGPUBuffer_D3D12* pASBuffer = (const CGPUBuffer_D3D12*)AS->pASBuffer;
        const CGPUBufferView_D3D12* pASBufferView = (const CGPUBufferView_D3D12*)AS->pASBufferView;
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
            .ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Format = DXGI_FORMAT_UNKNOWN,
            .RaytracingAccelerationStructure.Location = pASBuffer->mDxGpuAddress
        };
        DxDescriptorId srvId = pASBufferView->mDescriptorStartInCpuHeap + pASBufferView->mDxRawSrvOffset;
        D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = D3D12Util_DescriptorIdToCpuHandle(
            D->pCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], srvId);
        COM_CALL(CreateShaderResourceView, D->pDxDevice, CGPU_NULLPTR, &srvDesc, srvHandle);

        AS->super.scratch_buffer_size = (UINT)info.ScratchDataSizeInBytes;
        SkrCZoneEnd(zz);
    }

    // Create scratch buffer
    CGPUBufferDescriptor scratchBufferDesc = {
        .usages = CGPU_BUFFER_USAGE_SHADER_READWRITE,
        .memory_usage = CGPU_MEM_USAGE_GPU_ONLY,
        .start_state = CGPU_RESOURCE_STATE_COMMON,
        .size = AS->super.scratch_buffer_size,
        .name = "BuildTLAS-ScratchBuffer"
    };
    AS->pScratchBuffer = cgpu_create_buffer(device, &scratchBufferDesc);
    return &AS->super;
}

void cgpu_free_acceleration_structure_d3d12(CGPUAccelerationStructureId as)
{
    CGPUAccelerationStructure_D3D12* AS = (CGPUAccelerationStructure_D3D12*)as;
    if (AS->pASBuffer)
    {
        cgpu_free_buffer(AS->pASBuffer);
        AS->pASBuffer = CGPU_NULLPTR;
    }
    if (AS->pASBufferView)
    {
        cgpu_free_buffer_view(AS->pASBufferView);
        AS->pASBufferView = CGPU_NULLPTR;
    }
    if (AS->pScratchBuffer)
    {
        cgpu_free_buffer(AS->pScratchBuffer);
        AS->pScratchBuffer = CGPU_NULLPTR;
    }
    if (AS->super.type == CGPU_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL)
    {
        if (AS->asTop.pInstanceDescBuffer)
        {
            cgpu_free_buffer(AS->asTop.pInstanceDescBuffer);
            AS->asTop.pInstanceDescBuffer = CGPU_NULLPTR;
        }
    }
    if (AS->super.type == CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
    {
        if (AS->asBottom.pTransformBuffer)
        {
            cgpu_free_buffer(AS->asBottom.pTransformBuffer);
            AS->asBottom.pTransformBuffer = CGPU_NULLPTR;
        }
    }
    cgpu_free(AS);
}

void cgpu_cmd_build_acceleration_structures_d3d12(CGPUCommandBufferId cmd, const struct CGPUAccelerationStructureBuildDescriptor* desc)
{
    const CGPUCommandBuffer_D3D12* CMD = (const CGPUCommandBuffer_D3D12*)cmd;
    ID3D12GraphicsCommandList4* dxrCmd = NULL;
    COM_CALL(QueryInterface, CMD->pDxCmdList, IID_ARGS(ID3D12GraphicsCommandList4, &dxrCmd));
    cgpu_assert(dxrCmd);

    for (uint32_t i = 0; i < desc->as_count; i++)
    {
        CGPUAccelerationStructure_D3D12* AS = (CGPUAccelerationStructure_D3D12*)desc->as[i];
        const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE type = AS->mType;

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {
            .DestAccelerationStructureData = COM_CALL(GetGPUVirtualAddress, ((const CGPUBuffer_D3D12*)AS->pASBuffer)->pDxResource),
            .ScratchAccelerationStructureData = COM_CALL(GetGPUVirtualAddress, ((const CGPUBuffer_D3D12*)AS->pScratchBuffer)->pDxResource),
            .Inputs = {
                .Type = type,
                .DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
                .Flags = AS->mFlags,
                .pGeometryDescs = NULL,
                .NumDescs = AS->mDescCount,
            },
        };
        if (type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
        {
            buildDesc.Inputs.pGeometryDescs = AS->asBottom.pGeometryDescs;
        }
        else if (type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL)
        {
            buildDesc.Inputs.InstanceDescs = COM_CALL(GetGPUVirtualAddress, ((const CGPUBuffer_D3D12*)AS->asTop.pInstanceDescBuffer)->pDxResource);
        }
        AS->bIsDirty = true;

        COM_CALL(BuildRaytracingAccelerationStructure, dxrCmd, &buildDesc, 0, CGPU_NULLPTR);
    }

    if (desc->type == CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
    {
        const uint32_t STORE_COUNT = 4;
        SKR_DECLARE_ZERO_VLA(CGPUBufferBarrier, STORE, STORE_COUNT);
        const bool USE_STORE = desc->as_count < STORE_COUNT;

        CGPUBufferBarrier* BufferBarriers = USE_STORE ? STORE : cgpu_malloc(desc->as_count * sizeof(CGPUBufferBarrier));
        for (uint32_t i = 0; i < desc->as_count; i++)
        {
            CGPUAccelerationStructure_D3D12* AS = (CGPUAccelerationStructure_D3D12*)desc->as[i];
            if (AS->pASBuffer)
            {
                BufferBarriers[i].buffer = AS->pASBuffer;
                BufferBarriers[i].src_state = CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE;
                BufferBarriers[i].dst_state = CGPU_RESOURCE_STATE_ACCELERATION_STRUCTURE_READ;
            }
        }
        CGPUResourceBarrierDescriptor barriers = { .buffer_barriers = BufferBarriers, .buffer_barriers_count = desc->as_count };
        cgpu_cmd_resource_barrier(cmd, &barriers);

        if (!USE_STORE)
            cgpu_free(BufferBarriers);
    }

    SAFE_RELEASE(dxrCmd);
}

void cgpu_compute_encoder_bind_ray_pipeline_d3d12(CGPUComputePassEncoderId encoder, CGPURayPipelineId pipeline)
{
    CGPURayPipelineBase_D3D12* PPL = (CGPURayPipelineBase_D3D12*)pipeline;
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    D3D12Util_ResetRootSignature(Cmd, CGPU_PIPELINE_TYPE_COMPUTE, (const CGPURootSignature_D3D12*)pipeline->root_signature);
    COM_CALL(SetPipelineState1, (ID3D12GraphicsCommandList4*)Cmd->pDxCmdList, PPL->StateObject);
}

void cgpu_compute_encoder_dispatch_rays_d3d12(CGPUComputePassEncoderId encoder, const struct CGPUDispatchRaysDescriptor* desc)
{
    CGPURayPipelineBase_D3D12* PPL = (CGPURayPipelineBase_D3D12*)desc->pipeline;
    CGPUCommandBuffer_D3D12* Cmd = (CGPUCommandBuffer_D3D12*)encoder;
    SKR_DECLARE_ZERO(D3D12_DISPATCH_RAYS_DESC, rayDesc);
    rayDesc.Width = desc->width;
    rayDesc.Height = desc->height;
    rayDesc.Depth = desc->depth;
    rayDesc.RayGenerationShaderRecord = PPL->RayGenerationShaderRecord;
    rayDesc.MissShaderTable = PPL->MissShaderTable;
    rayDesc.HitGroupTable = PPL->HitGroupTable;
    rayDesc.CallableShaderTable = PPL->CallableShaderTable;
    COM_CALL(DispatchRays, (ID3D12GraphicsCommandList4*)Cmd->pDxCmdList, &rayDesc);
}

inline static D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE ToDXRASType(ECGPUAccelerationStructureType type)
{
    return type == CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
}

inline static D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS ToDXRBuildFlags(CGPUAccelerationStructureBuildFlags flags)
{
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS ret = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
    if (flags & CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION)
        ret |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
    if (flags & CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE)
        ret |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
    if (flags & CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY)
        ret |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY;
    if (flags & CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE)
        ret |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
    if (flags & CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD)
        ret |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
    if (flags & CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE)
        ret |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

    return ret;
}

inline static D3D12_RAYTRACING_GEOMETRY_FLAGS ToDXRGeometryFlags(CGPUAccelerationStructureGeometryFlags flags)
{
    D3D12_RAYTRACING_GEOMETRY_FLAGS ret = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
    if (flags & CGPU_ACCELERATION_STRUCTURE_GEOMETRY_FLAG_OPAQUE)
        ret |= D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    if (flags & CGPU_ACCELERATION_STRUCTURE_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION)
        ret |= D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;

    return ret;
}

inline static D3D12_RAYTRACING_INSTANCE_FLAGS ToDXRInstanceFlags(CGPUAccelerationStructureInstanceFlags flags)
{
    D3D12_RAYTRACING_INSTANCE_FLAGS ret = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
    if (flags & CGPU_ACCELERATION_STRUCTURE_INSTANCE_FLAG_FORCE_OPAQUE)
        ret |= D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE;
    if (flags & CGPU_ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE)
        ret |= D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE;
    if (flags & CGPU_ACCELERATION_STRUCTURE_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE)
        ret |= D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE;

    return ret;
}