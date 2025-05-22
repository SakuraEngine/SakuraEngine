#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrGuiRenderer/render/skr_render_window.hpp"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrCore/memory/memory.h"
#include "SkrGui/backend/canvas/canvas_types.hpp"

// constants
namespace skr::gui
{
#if _WIN32
static const ECGPUBackend kSKR_GUI_DEFAULT_RENDER_BACKEND = CGPU_BACKEND_D3D12;
#else
static const ECGPUBackend kSKR_GUI_DEFAULT_RENDER_BACKEND = CGPU_BACKEND_VULKAN;
#endif

static const auto kSKR_GUI_BACKBUFFER_FORMAT = CGPU_FORMAT_B8G8R8A8_UNORM;
} // namespace skr::gui

// pipeline helper
namespace skr::gui
{
inline static void read_bytes(const char8_t* file_name, char8_t** bytes, uint32_t* length)
{
    FILE* f = fopen((const char*)file_name, "rb");
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (char8_t*)malloc(*length);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

inline static void read_shader_bytes(const char8_t* virtual_path, uint32_t** bytes, uint32_t* length, ECGPUBackend backend)
{
    char8_t        shader_file[256];
    const char8_t* shader_path = SKR_UTF8("./../resources/shaders/");
    strcpy((char*)shader_file, (const char*)shader_path);
    strcat((char*)shader_file, (const char*)virtual_path);
    switch (backend)
    {
        case CGPU_BACKEND_VULKAN:
            strcat((char*)shader_file, ".spv");
            break;
        case CGPU_BACKEND_D3D12:
        case CGPU_BACKEND_XBOX_D3D12:
            strcat((char*)shader_file, ".dxil");
            break;
        default:
            break;
    }
    read_bytes(shader_file, (char8_t**)bytes, length);
}
} // namespace skr::gui

namespace skr::gui
{
// init & shutdown
void SkrRenderDevice::init()
{
    // create instance
    auto instance_desc                        = make_zeroed<CGPUInstanceDescriptor>();
    instance_desc.backend                     = kSKR_GUI_DEFAULT_RENDER_BACKEND;
    instance_desc.enable_debug_layer          = true;
    instance_desc.enable_gpu_based_validation = true;
    instance_desc.enable_set_name             = true;
    _cgpu_instance                            = cgpu_create_instance(&instance_desc);

    // filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(_cgpu_instance, CGPU_NULLPTR, &adapters_count);
    CGPUAdapterId adapters[64];
    cgpu_enum_adapters(_cgpu_instance, adapters, &adapters_count);
    _cgpu_adapter = adapters[0];

    // create device & queue
    auto queue_group_desc         = make_zeroed<CGPUQueueGroupDescriptor>();
    queue_group_desc.queue_type   = CGPU_QUEUE_TYPE_GRAPHICS;
    queue_group_desc.queue_count  = 1;
    auto device_desc              = make_zeroed<CGPUDeviceDescriptor>();
    device_desc.queue_groups      = &queue_group_desc;
    device_desc.queue_group_count = 1;
    _cgpu_device                  = cgpu_create_device(_cgpu_adapter, &device_desc);
    _cgpu_queue                   = cgpu_get_queue(_cgpu_device, CGPU_QUEUE_TYPE_GRAPHICS, 0);

    // create rg
    _render_graph = RenderGraph::create(
    [=, this](render_graph::RenderGraphBuilder& b) {
        b.with_device(_cgpu_device)
        .with_gfx_queue(_cgpu_queue);
    });

    // init vertex layout
    const uint32_t pos_offset      = static_cast<uint32_t>(offsetof(PaintVertex, position));
    const uint32_t texcoord_offset = static_cast<uint32_t>(offsetof(PaintVertex, texcoord));
    const uint32_t aa_offset       = static_cast<uint32_t>(offsetof(PaintVertex, aa));
    const uint32_t uv_offset       = static_cast<uint32_t>(offsetof(PaintVertex, clipUV));
    const uint32_t uv2_offset      = static_cast<uint32_t>(offsetof(PaintVertex, clipUV2));
    const uint32_t color_offset    = static_cast<uint32_t>(offsetof(PaintVertex, color));
    _vertex_layout.attributes[0]   = { u8"POSITION", 1, CGPU_FORMAT_R32G32B32A32_SFLOAT, 0, pos_offset, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
    _vertex_layout.attributes[1]   = { u8"TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, texcoord_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    _vertex_layout.attributes[2]   = { u8"AA", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, aa_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    _vertex_layout.attributes[3]   = { u8"UV", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, uv_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    _vertex_layout.attributes[4]   = { u8"UV_Two", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, uv2_offset, sizeof(skr_float2_t), CGPU_INPUT_RATE_VERTEX };
    _vertex_layout.attributes[5]   = { u8"COLOR", 1, CGPU_FORMAT_R8G8B8A8_UNORM, 0, color_offset, sizeof(skr_float4_t), CGPU_INPUT_RATE_VERTEX };
    _vertex_layout.attributes[6]   = { u8"TRANSFORM", 4, CGPU_FORMAT_R32G32B32A32_SFLOAT, 1, 0, sizeof(skr_float4x4_t), CGPU_INPUT_RATE_INSTANCE };
    _vertex_layout.attributes[7]   = { u8"PROJECTION", 4, CGPU_FORMAT_R32G32B32A32_SFLOAT, 2, 0, sizeof(skr_float4x4_t), CGPU_INPUT_RATE_INSTANCE };
    _vertex_layout.attributes[8]   = { u8"DRAW_DATA", 4, CGPU_FORMAT_R32G32B32A32_SFLOAT, 3, 0, sizeof(skr_float4x4_t), CGPU_INPUT_RATE_INSTANCE };
    _vertex_layout.attribute_count = 9;

    // create sampler
    CGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u             = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_v             = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_w             = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode           = CGPU_MIPMAP_MODE_NEAREST;
    sampler_desc.min_filter            = CGPU_FILTER_TYPE_NEAREST;
    sampler_desc.mag_filter            = CGPU_FILTER_TYPE_NEAREST;
    sampler_desc.compare_func          = CGPU_CMP_NEVER;
    _static_color_sampler              = cgpu_create_sampler(_cgpu_device, &sampler_desc);

    // create rs_pool
    CGPURootSignaturePoolDescriptor rs_pool_desc = {};
    rs_pool_desc.name                            = u8"GUI_RS_POOL";
    _rs_pool                                     = cgpu_create_root_signature_pool(_cgpu_device, &rs_pool_desc);

    // create vram service
    {
        auto ioServiceDesc       = make_zeroed<skr_vram_io_service_desc_t>();
        ioServiceDesc.name       = SKR_UTF8("GUI-VRAMService");
        ioServiceDesc.sleep_time = 1000 / 60;
        ioServiceDesc.gpu_device = _cgpu_device;
        _vram_service            = skr_io_vram_service_t::create(&ioServiceDesc);
        _vram_service->run();
    }
}
void SkrRenderDevice::shutdown()
{
    // destroy vram service
    if (_vram_service) skr_io_vram_service_t::destroy(_vram_service);

    // destroy rg
    if (_render_graph) RenderGraph::destroy(_render_graph);

    // destroy cgpu
    for (const auto& [flags, pipeline] : _pipelines)
    {
        auto rs = pipeline->root_signature;
        cgpu_free_render_pipeline(pipeline);
        cgpu_free_root_signature(rs);
    }
    if (_rs_pool) cgpu_free_root_signature_pool(_rs_pool);
    if (_static_color_sampler) cgpu_free_sampler(_static_color_sampler);
    if (_cgpu_queue) cgpu_free_queue(_cgpu_queue);
    if (_cgpu_device) cgpu_free_device(_cgpu_device);
    if (_cgpu_instance) cgpu_free_instance(_cgpu_instance);
}

// create view
SkrRenderWindow* SkrRenderDevice::create_window(SDL_Window* window)
{
    return SkrNew<SkrRenderWindow>(this, window);
}
void SkrRenderDevice::destroy_window(SkrRenderWindow* view)
{
    SkrDelete(view);
}

// pipeline
CGPURenderPipelineId SkrRenderDevice::get_pipeline(ESkrPipelineFlag flags, ECGPUSampleCount sample_count)
{
    SkrPipelineKey key = { flags, sample_count };
    if (auto _ = _pipelines.find(key)) return _.value();
    auto pipeline = create_pipeline(flags, sample_count);
    _pipelines.add(key, pipeline);
    return pipeline;
}
CGPURenderPipelineId SkrRenderDevice::create_pipeline(ESkrPipelineFlag flags, ECGPUSampleCount sample_count)
{
    const bool use_texture = flags & ESkrPipelineFlag_Textured;
    uint32_t * vs_bytes = nullptr, vs_length = 0;
    uint32_t * fs_bytes = nullptr, fs_length = 0;
    read_shader_bytes(SKR_UTF8("GUI/vertex"), &vs_bytes, &vs_length, _cgpu_device->adapter->instance->backend);
    if (use_texture)
    {
        read_shader_bytes(SKR_UTF8("GUI/pixel2"), &fs_bytes, &fs_length, _cgpu_device->adapter->instance->backend);
    }
    else
    {
        read_shader_bytes(SKR_UTF8("GUI/pixel"), &fs_bytes, &fs_length, _cgpu_device->adapter->instance->backend);
    }
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.stage                       = CGPU_SHADER_STAGE_VERT;
    vs_desc.name                        = SKR_UTF8("VertexShaderLibrary");
    vs_desc.code                        = vs_bytes;
    vs_desc.code_size                   = vs_length;
    CGPUShaderLibraryDescriptor ps_desc = {};
    ps_desc.name                        = SKR_UTF8("FragmentShaderLibrary");
    ps_desc.stage                       = CGPU_SHADER_STAGE_FRAG;
    ps_desc.code                        = fs_bytes;
    ps_desc.code_size                   = fs_length;
    CGPUShaderLibraryId vertex_shader   = cgpu_create_shader_library(_cgpu_device, &vs_desc);
    CGPUShaderLibraryId fragment_shader = cgpu_create_shader_library(_cgpu_device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGPUShaderEntryDescriptor ppl_shaders[2];
    ppl_shaders[0].stage   = CGPU_SHADER_STAGE_VERT;
    ppl_shaders[0].entry   = SKR_UTF8("main");
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage   = CGPU_SHADER_STAGE_FRAG;
    ppl_shaders[1].entry   = SKR_UTF8("main");
    ppl_shaders[1].library = fragment_shader;

    const char8_t*              static_sampler_name = u8"color_sampler";
    CGPURootSignatureDescriptor rs_desc             = {};
    rs_desc.shaders                                 = ppl_shaders;
    rs_desc.shader_count                            = 2;
    rs_desc.pool                                    = _rs_pool;
    if ((!(flags & ESkrPipelineFlag_CustomSampler)) && use_texture)
    {
        rs_desc.static_sampler_count = 1;
        rs_desc.static_sampler_names = &static_sampler_name;
        rs_desc.static_samplers      = &_static_color_sampler;
    }
    auto root_sig = cgpu_create_root_signature(_cgpu_device, &rs_desc);

    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature               = root_sig;
    rp_desc.prim_topology                = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout                = &_vertex_layout;
    rp_desc.vertex_shader                = &ppl_shaders[0];
    rp_desc.fragment_shader              = &ppl_shaders[1];
    rp_desc.render_target_count          = 1;
    rp_desc.color_formats                = &kSKR_GUI_BACKBUFFER_FORMAT;
    rp_desc.depth_stencil_format         = CGPU_FORMAT_D32_SFLOAT;

    CGPURasterizerStateDescriptor rs_state = {};
    rs_state.cull_mode                     = CGPU_CULL_MODE_NONE;
    rs_state.fill_mode                     = CGPU_FILL_MODE_SOLID;
    rs_state.front_face                    = CGPU_FRONT_FACE_CCW;
    rs_state.slope_scaled_depth_bias       = 0.f;
    rs_state.enable_depth_clamp            = false;
    rs_state.enable_scissor                = true;
    rs_state.enable_multi_sample           = false;
    rs_state.depth_bias                    = 0;
    rp_desc.rasterizer_state               = &rs_state;

    CGPUDepthStateDescriptor depth_state = {};
    depth_state.depth_test               = flags & ESkrPipelineFlag_TestZ;
    depth_state.depth_func               = depth_state.depth_test ? CGPU_CMP_LEQUAL : CGPU_CMP_NEVER;
    depth_state.depth_write              = flags & ESkrPipelineFlag_WriteZ;
    rp_desc.depth_state                  = &depth_state;

    CGPUBlendStateDescriptor blend_state = {};
    for (uint32_t i = 0; i < 1; i++)
    {
        blend_state.blend_modes[i]       = CGPU_BLEND_MODE_ADD;
        blend_state.blend_alpha_modes[i] = CGPU_BLEND_MODE_ADD;
        blend_state.masks[i]             = CGPU_COLOR_MASK_ALL;

        blend_state.src_factors[i]       = CGPU_BLEND_CONST_SRC_ALPHA;
        blend_state.dst_factors[i]       = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
        blend_state.src_alpha_factors[i] = CGPU_BLEND_CONST_ONE;
        blend_state.dst_alpha_factors[i] = CGPU_BLEND_CONST_ZERO;
    }
    rp_desc.blend_state = &blend_state;

    rp_desc.sample_count = sample_count;

    auto pipeline = cgpu_create_render_pipeline(_cgpu_device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
    return pipeline;
}
} // namespace skr::gui

// bool validateAttributes(GDIRendererPipelineAttributes attributes)
// {
//     const bool use_custom_sampler = attributes & GDI_RENDERER_PIPELINE_ATTRIBUTE_CUSTOM_SAMPLER;
//     const bool use_texture = attributes & GDI_RENDERER_PIPELINE_ATTRIBUTE_TEXTURED;
//     if (use_custom_sampler && !use_texture) return false;
//     return true;
// }

// void GDIRenderer_RenderGraph::updatePendingTextures(skr::render_graph::RenderGraph* graph) SKR_NOEXCEPT
// {
//     const auto                    frame_index = graph->get_frame_index();
//     GDITextureUpdate_RenderGraph* update = nullptr;
//     // 1. filter request_updates to pending_updates
//     skr::Vector_map<IGDITexture*, GDITextureUpdate_RenderGraph*> filter_map;
//     while (request_updates.try_dequeue(update))
//     {
//         auto iter = filter_map.find(update->texture);
//         if (iter != filter_map.end())
//         {
//             atomic_store_relaxed(&iter->second->state, (uint32_t)EGDIResourceState::Okay);
//         }
//         filter_map[update->texture] = update;
//     }
//     for (auto& [id, update] : filter_map)
//     {
//         pending_updates.enqueue(update);
//     }
//     // 2. filter pending_updates to copies
//     skr::FixedVector<GDITextureUpdate_RenderGraph*, 8> copies;
//     skr::FixedVector<GDITextureUpdate_RenderGraph*, 8> pending;
//     while (pending_updates.try_dequeue(update))
//     {
//         if (update->texture->get_state() == EGDIResourceState::Okay)
//         {
//             if (update->get_state() == EGDIResourceState::Requested)
//             {
//                 update->upload_buffer = graph->create_buffer(
//                 [update](auto& g, auto& builder) {
//                     builder.size(update->image->get_data().size())
//                     .with_tags(kRenderGraphDynamicResourceTag)
//                     .as_upload_buffer();
//                 });
//                 update->texture_handle = graph->create_texture(
//                 [update](auto& g, auto& builder) {
//                     GDITexture_RenderGraph* T = (GDITexture_RenderGraph*)update->texture;
//                     builder.import(T->texture, CGPU_RESOURCE_STATE_SHADER_RESOURCE);
//                 });
//                 copies.emplace_back(update);
//                 continue;
//             }
//             else if (update->get_state() == EGDIResourceState::Loading)
//             {
//                 if (frame_index > update->execute_frame_index + RG_MAX_FRAME_IN_FLIGHT)
//                 {
//                     atomic_store_relaxed(&update->state, (uint32_t)EGDIResourceState::Okay);
//                 }
//             }
//         }
//         if (update->get_state() != EGDIResourceState::Okay)
//         {
//             pending.emplace_back(update);
//         }
//     }
//     for (auto update : pending)
//     {
//         pending_updates.enqueue(update);
//     }

//     graph->add_copy_pass(
//     [&](render_graph::RenderGraph& g, render_graph::CopyPassBuilder& builder) {
//         SkrZoneScopedN("UpdateTextures");
//         builder.set_name(u8"gdi_texture_update_pass")
//         .can_be_lone();
//         for (auto copy : copies)
//         {
//             builder.buffer_to_texture(copy->upload_buffer.range(0, 0), copy->texture_handle, CGPU_RESOURCE_STATE_SHADER_RESOURCE);
//         }
//     },
//     [=](render_graph::RenderGraph& g, render_graph::CopyPassContext& context) {
//         for (auto copy : copies)
//         {
//             auto       buffer = context.resolve(copy->upload_buffer);
//             const auto data = copy->image->get_data();
//             ::memcpy(buffer->info->cpu_mapped_address, data.data(), data.size());
//         }
//     });
// }
