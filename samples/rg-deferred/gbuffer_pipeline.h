#pragma once
#include "math/vectormath.hpp"
#include "../common/utils.h"

const ECGpuFormat gbuffer_formats[] = {
    PF_R8G8B8A8_UNORM, PF_R16G16B16A16_SNORM
};
const ECGpuFormat gbuffer_depth_format = PF_D32_SFLOAT;

inline CGpuRenderPipelineId create_gbuffer_render_pipeline(CGpuDeviceId device)
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("rg-deferred/gbuffer_vs", &vs_bytes, &vs_length,
        device->adapter->instance->backend);
    read_shader_bytes("rg-deferred/gbuffer_fs", &fs_bytes, &fs_length,
        device->adapter->instance->backend);
    CGpuShaderLibraryDescriptor vs_desc = {};
    vs_desc.stage = SHADER_STAGE_VERT;
    vs_desc.name = "GBufferVertexShader";
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    CGpuShaderLibraryDescriptor ps_desc = {};
    ps_desc.name = "GBufferFragmentShader";
    ps_desc.stage = SHADER_STAGE_FRAG;
    ps_desc.code = fs_bytes;
    ps_desc.code_size = fs_length;
    CGpuShaderLibraryId gbuffer_vs = cgpu_create_shader_library(device, &vs_desc);
    CGpuShaderLibraryId gbuffer_fs = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGpuPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = SHADER_STAGE_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = gbuffer_vs;
    ppl_shaders[1].stage = SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = gbuffer_fs;
    CGpuRootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    auto gbuffer_root_sig = cgpu_create_root_signature(device, &rs_desc);
    namespace smath = sakura::math;
    CGpuVertexLayout vertex_layout = {};
    vertex_layout.attributes[0] = { "POSITION", 1, PF_R32G32B32_SFLOAT, 0, 0, sizeof(smath::Vector3f), INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, PF_R32G32_SFLOAT, 1, 0, sizeof(smath::Vector2f), INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "NORMAL", 1, PF_R8G8B8A8_SNORM, 2, 0, sizeof(uint32_t), INPUT_RATE_VERTEX };
    vertex_layout.attributes[3] = { "TANGENT", 1, PF_R8G8B8A8_SNORM, 3, 0, sizeof(uint32_t), INPUT_RATE_VERTEX };
    vertex_layout.attributes[4] = { "MODEL", 4, PF_R32G32B32A32_SFLOAT, 4, 0, sizeof(smath::float4x4), INPUT_RATE_INSTANCE };
    vertex_layout.attribute_count = 5;
    CGpuRenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = gbuffer_root_sig;
    rp_desc.prim_topology = PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = sizeof(gbuffer_formats) / sizeof(ECGpuFormat);
    rp_desc.color_formats = gbuffer_formats;
    rp_desc.depth_stencil_format = gbuffer_depth_format;
    CGpuRasterizerStateDescriptor raster_desc = {};
    raster_desc.cull_mode = ECGpuCullMode::CULL_MODE_BACK;
    raster_desc.depth_bias = 0;
    raster_desc.fill_mode = FILL_MODE_SOLID;
    raster_desc.front_face = FRONT_FACE_CCW;
    rp_desc.rasterizer_state = &raster_desc;
    CGpuDepthStateDescriptor ds_desc = {};
    ds_desc.depth_func = CMP_LEQUAL;
    ds_desc.depth_write = true;
    ds_desc.depth_test = true;
    rp_desc.depth_state = &ds_desc;
    auto gbuffer_pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(gbuffer_vs);
    cgpu_free_shader_library(gbuffer_fs);
    return gbuffer_pipeline;
}