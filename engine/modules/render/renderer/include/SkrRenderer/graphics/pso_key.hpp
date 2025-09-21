#pragma once
#include "SkrBase/atomic/atomic.h"
#include "SkrRenderer/graphics/pso_map.hpp"

namespace skr
{

struct SKR_RENDERER_API PSOMapKey
{
    PSOMapKey(const CGPURenderPipelineDescriptor& desc, uint64_t frame) SKR_NOEXCEPT;
    ~PSOMapKey() SKR_NOEXCEPT;

    CGPURootSignatureId root_signature;
    CGPUShaderEntryDescriptor vertex_shader;
    CGPUShaderEntryDescriptor tesc_shader;
    CGPUShaderEntryDescriptor tese_shader;
    CGPUShaderEntryDescriptor geom_shader;
    CGPUShaderEntryDescriptor fragment_shader;
    CGPUVertexLayout vertex_layout;
    CGPUBlendStateDescriptor blend_state;
    CGPUDepthStateDescriptor depth_state;
    CGPURasterizerStateDescriptor rasterizer_state;
    ECGPUFormat color_formats[CGPU_MAX_MRT_COUNT];

    CGPURenderPipelineDescriptor descriptor;

    SAtomicU64 frame = UINT64_MAX;
    SAtomicU64 pso_frame = UINT64_MAX;
    SAtomicU32 rc = 0;
    SAtomicU32 pso_rc = 0;
    SAtomicU32 pso_status = SKR_PSO_MAP_PSO_STATUS_UNINSTALLED;
    CGPURenderPipelineId pso = nullptr;
};

} // namespace skr