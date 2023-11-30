#pragma once
#include "SkrRenderer/pso_map.h"
#include "SkrRT/platform/atomic.h"
#include "SkrRT/containers_new/array.hpp"

namespace skr {
namespace renderer {

struct PSOMapKey
{
    PSOMapKey(const CGPURenderPipelineDescriptor& desc, uint64_t frame) SKR_NOEXCEPT;
    ~PSOMapKey() SKR_NOEXCEPT;

    CGPURootSignatureId root_signature;
    CGPUShaderEntryDescriptor vertex_shader;
    skr::Array<CGPUConstantSpecialization> vertex_specializations;
    CGPUShaderEntryDescriptor tesc_shader;
    skr::Array<CGPUConstantSpecialization> tesc_specializations;
    CGPUShaderEntryDescriptor tese_shader;
    skr::Array<CGPUConstantSpecialization> tese_specializations;
    CGPUShaderEntryDescriptor geom_shader;
    skr::Array<CGPUConstantSpecialization> geom_specializations;
    CGPUShaderEntryDescriptor fragment_shader;
    skr::Array<CGPUConstantSpecialization> fragment_specializations;
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

}
}