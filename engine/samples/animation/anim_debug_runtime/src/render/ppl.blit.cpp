#include "AnimDebugRuntime/renderer.h"
#include "common/utils.h"
namespace animd
{

void Renderer::create_blit_pipeline()
{
    auto _device = _render_device->get_cgpu_device();
    auto _static_sampler = _render_device->get_linear_sampler();
    ECGPUFormat output_format = CGPU_FORMAT_R8G8B8A8_UNORM;
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes(SKR_UTF8("AnimDebug/screen_vs"), &vs_bytes, &vs_length, _device->adapter->instance->backend);
    read_shader_bytes(SKR_UTF8("AnimDebug/blit_fs"), &fs_bytes, &fs_length, _device->adapter->instance->backend);
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = SKR_UTF8("ScreenVertexShader");
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    CGPUShaderLibraryDescriptor ps_desc = {};
    ps_desc.name = SKR_UTF8("BlitFragmentShader");
    ps_desc.code = fs_bytes;
    ps_desc.code_size = fs_length;
    auto screen_vs = cgpu_create_shader_library(_device, &vs_desc);
    auto blit_fs = cgpu_create_shader_library(_device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGPUShaderEntryDescriptor ppl_shaders[2];
    ppl_shaders[0].entry = SKR_UTF8("main");
    ppl_shaders[0].library = screen_vs;
    ppl_shaders[1].entry = SKR_UTF8("main");
    ppl_shaders[1].library = blit_fs;
    const char8_t* static_sampler_name = SKR_UTF8("texture_sampler");
    CGPURootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    rs_desc.static_sampler_count = 1;
    rs_desc.static_sampler_names = &static_sampler_name;
    rs_desc.static_samplers = &_static_sampler;
    auto lighting_root_sig = cgpu_create_root_signature(_device, &rs_desc);
    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attribute_count = 0;
    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = lighting_root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 1;
    rp_desc.color_formats = &output_format;
    _blit_pipeline = cgpu_create_render_pipeline(_device, &rp_desc);
    cgpu_free_shader_library(screen_vs);
    cgpu_free_shader_library(blit_fs);
}
} // namespace animd