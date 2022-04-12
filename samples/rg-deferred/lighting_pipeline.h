#pragma once
#include "math/vectormath.hpp"
#include "../common/utils.h"

const ECGpuFormat lighting_buffer_format = PF_R16G16B16A16_SFLOAT;

CGpuComputePipelineId create_lighting_compute_pipeline(CGpuDeviceId device)
{
    uint32_t *cs_bytes, cs_length;
    read_shader_bytes("rg-deferred/lighting_cs", &cs_bytes, &cs_length,
        device->adapter->instance->backend);
    CGpuShaderLibraryDescriptor cs_desc = {};
    cs_desc.name = "LightingComputeShader";
    cs_desc.stage = SHADER_STAGE_COMPUTE;
    cs_desc.code = cs_bytes;
    cs_desc.code_size = cs_length;
    CGpuShaderLibraryId lighting_cs = cgpu_create_shader_library(device, &cs_desc);
    free(cs_bytes);
    CGpuPipelineShaderDescriptor pipeline_cs = {};
    pipeline_cs.stage = SHADER_STAGE_COMPUTE;
    pipeline_cs.entry = "main";
    pipeline_cs.library = lighting_cs;
    const char8_t* root_constant_name = "root_constants";
    CGpuRootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = &pipeline_cs;
    rs_desc.shader_count = 1;
    rs_desc.root_constant_count = 1;
    rs_desc.root_constant_names = &root_constant_name;
    auto lighting_cs_root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGpuVertexLayout vertex_layout = {};
    vertex_layout.attribute_count = 0;
    CGpuComputePipelineDescriptor cp_desc = {};
    cp_desc.compute_shader = &pipeline_cs;
    cp_desc.root_signature = lighting_cs_root_sig;
    auto lighting_cs_pipeline = cgpu_create_compute_pipeline(device, &cp_desc);
    cgpu_free_shader_library(lighting_cs);
    return lighting_cs_pipeline;
}

CGpuRenderPipelineId create_lighting_render_pipeline(CGpuDeviceId device, CGpuSamplerId static_sampler, ECGpuFormat format)
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("rg-deferred/screen_vs", &vs_bytes, &vs_length,
        device->adapter->instance->backend);
    read_shader_bytes("rg-deferred/lighting_fs", &fs_bytes, &fs_length,
        device->adapter->instance->backend);
    CGpuShaderLibraryDescriptor vs_desc = {};
    vs_desc.stage = SHADER_STAGE_VERT;
    vs_desc.name = "ScreenVertexShader";
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    CGpuShaderLibraryDescriptor ps_desc = {};
    ps_desc.name = "LightingFragmentShader";
    ps_desc.stage = SHADER_STAGE_FRAG;
    ps_desc.code = fs_bytes;
    ps_desc.code_size = fs_length;
    auto screen_vs = cgpu_create_shader_library(device, &vs_desc);
    auto lighting_fs = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGpuPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = SHADER_STAGE_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = screen_vs;
    ppl_shaders[1].stage = SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = lighting_fs;
    const char8_t* root_constant_name = "root_constants";
    const char8_t* static_sampler_name = "texture_sampler";
    CGpuRootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    rs_desc.root_constant_count = 1;
    rs_desc.root_constant_names = &root_constant_name;
    rs_desc.static_sampler_count = 1;
    rs_desc.static_sampler_names = &static_sampler_name;
    rs_desc.static_samplers = &static_sampler;
    auto lighting_root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGpuVertexLayout vertex_layout = {};
    vertex_layout.attribute_count = 0;
    CGpuRenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = lighting_root_sig;
    rp_desc.prim_topology = PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 1;
    auto backbuffer_format = format;
    rp_desc.color_formats = &backbuffer_format;
    auto lighting_pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(screen_vs);
    cgpu_free_shader_library(lighting_fs);
    return lighting_pipeline;
}
