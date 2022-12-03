#pragma once
#include "../../cgpu/common/utils.h"
#include "utils/types.h"

const ECGPUFormat gbuffer_formats[] = {
    CGPU_FORMAT_R8G8B8A8_UNORM, CGPU_FORMAT_R16G16B16A16_SNORM
};
const ECGPUFormat gbuffer_depth_format = CGPU_FORMAT_D32_SFLOAT;

inline CGPURenderPipelineId create_gbuffer_render_pipeline(CGPUDeviceId device)
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("rg-deferred/gbuffer_vs", &vs_bytes, &vs_length,
    device->adapter->instance->backend);
    read_shader_bytes("rg-deferred/gbuffer_fs", &fs_bytes, &fs_length,
    device->adapter->instance->backend);
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.name = "GBufferVertexShader";
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    CGPUShaderLibraryDescriptor ps_desc = {};
    ps_desc.name = "GBufferFragmentShader";
    ps_desc.stage = CGPU_SHADER_STAGE_FRAG;
    ps_desc.code = fs_bytes;
    ps_desc.code_size = fs_length;
    CGPUShaderLibraryId gbuffer_vs = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId gbuffer_fs = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGPUPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = CGPU_SHADER_STAGE_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = gbuffer_vs;
    ppl_shaders[1].stage = CGPU_SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = gbuffer_fs;
    CGPURootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    rs_desc.push_constant_count = 1;
    const char* root_const_name = "push_constants";
    rs_desc.push_constant_names = &root_const_name;
    auto gbuffer_root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(skr_float3_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 1, 0, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "NORMAL", 1, CGPU_FORMAT_R8G8B8A8_SNORM, 2, 0, sizeof(uint32_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[3] = { "TANGENT", 1, CGPU_FORMAT_R8G8B8A8_SNORM, 3, 0, sizeof(uint32_t), CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[4] = { "MODEL", 4, CGPU_FORMAT_R32G32B32A32_SFLOAT, 4, 0, sizeof(skr_float4x4_t), CGPU_INPUT_RATE_INSTANCE };
    vertex_layout.attribute_count = 5;
    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = gbuffer_root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = sizeof(gbuffer_formats) / sizeof(ECGPUFormat);
    rp_desc.color_formats = gbuffer_formats;
    rp_desc.depth_stencil_format = gbuffer_depth_format;
    CGPURasterizerStateDescriptor raster_desc = {};
    raster_desc.cull_mode = ECGPUCullMode::CGPU_CULL_MODE_BACK;
    raster_desc.depth_bias = 0;
    raster_desc.fill_mode = CGPU_FILL_MODE_SOLID;
    raster_desc.front_face = CGPU_FRONT_FACE_CCW;
    rp_desc.rasterizer_state = &raster_desc;
    CGPUDepthStateDescriptor ds_desc = {};
    ds_desc.depth_func = CGPU_CMP_LEQUAL;
    ds_desc.depth_write = true;
    ds_desc.depth_test = true;
    rp_desc.depth_state = &ds_desc;
    auto gbuffer_pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(gbuffer_vs);
    cgpu_free_shader_library(gbuffer_fs);
    return gbuffer_pipeline;
}