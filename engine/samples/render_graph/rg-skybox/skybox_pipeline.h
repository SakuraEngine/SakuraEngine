#pragma once
#include "common/utils.h"

inline CGPUComputePipelineId create_skybox_compute_pipeline(CGPUDeviceId device, CGPUSamplerId static_sampler)
{
    uint32_t *cs_bytes, cs_length;
    read_shader_bytes(SKR_UTF8("rg-skybox/skybox.cs"), &cs_bytes, &cs_length, device->adapter->instance->backend);
    CGPUShaderLibraryDescriptor cs_desc = {};
    cs_desc.name = SKR_UTF8("LightingComputeShader");
    cs_desc.code = cs_bytes;
    cs_desc.code_size = cs_length;
    CGPUShaderLibraryId lighting_cs = cgpu_create_shader_library(device, &cs_desc);
    free(cs_bytes);
    CGPUShaderEntryDescriptor pipeline_cs = {};
    pipeline_cs.entry = SKR_UTF8("cs");
    pipeline_cs.library = lighting_cs;
    const char8_t* push_constant_name = SKR_UTF8("push_constants");
    const char8_t* static_sampler_name = SKR_UTF8("sky_sampler");
    CGPURootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = &pipeline_cs;
    rs_desc.shader_count = 1;
    rs_desc.push_constant_count = 1;
    rs_desc.push_constant_names = &push_constant_name;
    rs_desc.static_sampler_count = 1;
    rs_desc.static_sampler_names = &static_sampler_name;
    rs_desc.static_samplers = &static_sampler;
    auto lighting_cs_root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attribute_count = 0;
    CGPUComputePipelineDescriptor cp_desc = {};
    cp_desc.compute_shader = &pipeline_cs;
    cp_desc.root_signature = lighting_cs_root_sig;
    auto lighting_cs_pipeline = cgpu_create_compute_pipeline(device, &cp_desc);
    cgpu_free_shader_library(lighting_cs);
    return lighting_cs_pipeline;
}