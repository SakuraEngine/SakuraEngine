#include "SkrRenderer/pso_key.hpp"

namespace skr {
namespace renderer {

PSOMapKey::PSOMapKey(const CGPURenderPipelineDescriptor& desc, uint64_t frame) SKR_NOEXCEPT
    : root_signature(desc.root_signature->pool_sig ? desc.root_signature->pool_sig : desc.root_signature), 
    frame(frame), rc(0), pso_rc(0)
{
    descriptor.root_signature = root_signature;
    if (desc.vertex_shader)
    {
        vertex_shader = *desc.vertex_shader;
        vertex_specializations = skr::vector<CGPUConstantSpecialization>(desc.vertex_shader->constants, desc.vertex_shader->constants + desc.vertex_shader->num_constants);
        descriptor.vertex_shader = &vertex_shader;
    }
    if (desc.tesc_shader)
    {
        tesc_shader = *desc.tesc_shader;
        tesc_specializations = skr::vector<CGPUConstantSpecialization>(desc.tesc_shader->constants, desc.tesc_shader->constants + desc.tesc_shader->num_constants);
        descriptor.tesc_shader = &tesc_shader;
    }
    if (desc.tese_shader)
    {
        tese_shader = *desc.tese_shader;
        tese_specializations = skr::vector<CGPUConstantSpecialization>(desc.tese_shader->constants, desc.tese_shader->constants + desc.tese_shader->num_constants);
        descriptor.tese_shader = &tese_shader;
    }
    if (desc.geom_shader)
    {
        geom_shader = *desc.geom_shader;
        geom_specializations = skr::vector<CGPUConstantSpecialization>(desc.geom_shader->constants, desc.geom_shader->constants + desc.geom_shader->num_constants);
        descriptor.geom_shader = &geom_shader;
    }
    if (desc.fragment_shader)
    {
        fragment_shader = *desc.fragment_shader;
        fragment_specializations = skr::vector<CGPUConstantSpecialization>(desc.fragment_shader->constants, desc.fragment_shader->constants + desc.fragment_shader->num_constants);
        descriptor.fragment_shader = &fragment_shader;
    }
    if (desc.vertex_layout) vertex_layout = *desc.vertex_layout;
    descriptor.vertex_layout = &vertex_layout;

    if (desc.blend_state) blend_state = *desc.blend_state;
    descriptor.blend_state = &blend_state;

    if (desc.depth_state) depth_state = *desc.depth_state;
    descriptor.depth_state = &depth_state;

    if (desc.rasterizer_state) rasterizer_state = *desc.rasterizer_state;
    descriptor.rasterizer_state = &rasterizer_state;

    for (uint32_t i = 0; i < CGPU_MAX_MRT_COUNT; ++i)
    {
        color_formats[i] = CGPU_FORMAT_UNDEFINED;
    }
    for (uint32_t i = 0; i < desc.render_target_count; ++i)
    {
        color_formats[i] = desc.color_formats[i];
    }
    descriptor.color_formats = color_formats;
    descriptor.render_target_count = desc.render_target_count;
    descriptor.sample_count = desc.sample_count;
    descriptor.sample_quality = desc.sample_quality;
    descriptor.color_resolve_disable_mask = desc.color_resolve_disable_mask;
    descriptor.depth_stencil_format = desc.depth_stencil_format;
    descriptor.prim_topology = desc.prim_topology;
    descriptor.enable_indirect_command = desc.enable_indirect_command;
}

PSOMapKey::~PSOMapKey() SKR_NOEXCEPT
{
    root_signature = nullptr;
}

} // namespace renderer
} // namespace skr