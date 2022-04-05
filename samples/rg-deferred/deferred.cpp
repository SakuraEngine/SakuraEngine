#include "cube_geometry.h"
#include "../common/utils.h"
#include "render_graph/frontend/render_graph.hpp"

thread_local SDL_Window* sdl_window;
thread_local SDL_SysWMinfo wmInfo;
thread_local CGpuSurfaceId surface;
thread_local CGpuSwapChainId swapchain;
thread_local uint32_t backbuffer_index;
thread_local CGpuFenceId present_fence;

CubeGeometry::InstanceData CubeGeometry::instance_data;
ECGpuFormat gbuffer_formats[] = { PF_R8G8B8A8_UNORM, PF_R16G16B16A16_SNORM };

#if _WINDOWS
thread_local ECGpuBackend backend = CGPU_BACKEND_D3D12;
#else
thread_local ECGpuBackend backend = CGPU_BACKEND_VULKAN;
#endif

thread_local CGpuInstanceId instance;
thread_local CGpuAdapterId adapter;
thread_local CGpuDeviceId device;
thread_local CGpuQueueId gfx_queue;
thread_local CGpuSamplerId static_sampler;

thread_local CGpuBufferId index_buffer;
thread_local CGpuBufferId vertex_buffer;
thread_local CGpuBufferId instance_buffer;

thread_local CGpuRootSignatureId gbuffer_root_sig;
thread_local CGpuRenderPipelineId gbuffer_pipeline;

thread_local CGpuRootSignatureId lighting_root_sig;
thread_local CGpuRenderPipelineId lighting_pipeline;

void create_api_objects()
{
    // Create instance
    CGpuInstanceDescriptor instance_desc = {};
    instance_desc.backend = backend;
    instance_desc.enable_debug_layer = true;
    instance_desc.enable_gpu_based_validation = true;
    instance_desc.enable_set_name = true;
    instance = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    CGpuAdapterId adapters[64];
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    // Create device
    CGpuQueueGroupDescriptor queue_group_desc = {};
    queue_group_desc.queueType = QUEUE_TYPE_GRAPHICS;
    queue_group_desc.queueCount = 1;
    CGpuDeviceDescriptor device_desc = {};
    device_desc.queueGroups = &queue_group_desc;
    device_desc.queueGroupCount = 1;
    device = cgpu_create_device(adapter, &device_desc);
    gfx_queue = cgpu_get_queue(device, QUEUE_TYPE_GRAPHICS, 0);
    present_fence = cgpu_create_fence(device);
    // Sampler
    CGpuSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u = ADDRESS_MODE_REPEAT;
    sampler_desc.address_v = ADDRESS_MODE_REPEAT;
    sampler_desc.address_w = ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode = MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter = FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter = FILTER_TYPE_LINEAR;
    sampler_desc.compare_func = CMP_NEVER;
    static_sampler = cgpu_create_sampler(device, &sampler_desc);

    // Create swapchain
#if defined(_WIN32) || defined(_WIN64)
    surface = cgpu_surface_from_hwnd(device, wmInfo.info.win.window);
#elif defined(__APPLE__)
    struct CGpuNSView* ns_view = (struct CGpuNSView*)nswindow_get_content_view(wmInfo.info.cocoa.window);
    surface = cgpu_surface_from_ns_view(device, ns_view);
#endif
    CGpuSwapChainDescriptor chain_desc = {};
    chain_desc.presentQueues = &gfx_queue;
    chain_desc.presentQueuesCount = 1;
    chain_desc.width = BACK_BUFFER_WIDTH;
    chain_desc.height = BACK_BUFFER_HEIGHT;
    chain_desc.surface = surface;
    chain_desc.imageCount = 3;
    chain_desc.format = PF_R8G8B8A8_UNORM;
    chain_desc.enableVsync = true;
    swapchain = cgpu_create_swapchain(device, &chain_desc);
    // upload
    CGpuBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name = "UploadBuffer";
    upload_buffer_desc.flags = BCF_OWN_MEMORY_BIT | BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors = RT_NONE;
    upload_buffer_desc.memory_usage = MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size = sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices) + sizeof(CubeGeometry::InstanceData);
    auto upload_buffer = cgpu_create_buffer(device, &upload_buffer_desc);
    CGpuBufferDescriptor vb_desc = {};
    vb_desc.name = "VertexBuffer";
    vb_desc.flags = BCF_OWN_MEMORY_BIT;
    vb_desc.descriptors = RT_VERTEX_BUFFER;
    vb_desc.memory_usage = MEM_USAGE_GPU_ONLY;
    vb_desc.size = sizeof(CubeGeometry);
    vertex_buffer = cgpu_create_buffer(device, &vb_desc);
    CGpuBufferDescriptor ib_desc = {};
    ib_desc.name = "IndexBuffer";
    ib_desc.flags = BCF_OWN_MEMORY_BIT;
    ib_desc.descriptors = RT_INDEX_BUFFER;
    ib_desc.memory_usage = MEM_USAGE_GPU_ONLY;
    ib_desc.size = sizeof(CubeGeometry::g_Indices);
    index_buffer = cgpu_create_buffer(device, &ib_desc);
    CGpuBufferDescriptor inb_desc = {};
    inb_desc.name = "InstanceBuffer";
    inb_desc.flags = BCF_OWN_MEMORY_BIT;
    inb_desc.descriptors = RT_VERTEX_BUFFER;
    inb_desc.memory_usage = MEM_USAGE_GPU_ONLY;
    inb_desc.size = sizeof(CubeGeometry::InstanceData);
    instance_buffer = cgpu_create_buffer(device, &inb_desc);
    auto pool_desc = CGpuCommandPoolDescriptor();
    auto cmd_pool = cgpu_create_command_pool(gfx_queue, &pool_desc);
    auto cmd_desc = CGpuCommandBufferDescriptor();
    auto cpy_cmd = cgpu_create_command_buffer(cmd_pool, &cmd_desc);
    {
        auto geom = CubeGeometry();
        memcpy(upload_buffer->cpu_mapped_address, &geom, upload_buffer_desc.size);
    }
    cgpu_cmd_begin(cpy_cmd);
    CGpuBufferToBufferTransfer vb_cpy = {};
    vb_cpy.dst = vertex_buffer;
    vb_cpy.dst_offset = 0;
    vb_cpy.src = upload_buffer;
    vb_cpy.src_offset = 0;
    vb_cpy.size = sizeof(CubeGeometry);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &vb_cpy);
    {
        memcpy((char8_t*)upload_buffer->cpu_mapped_address + sizeof(CubeGeometry),
            CubeGeometry::g_Indices, sizeof(CubeGeometry::g_Indices));
    }
    CGpuBufferToBufferTransfer ib_cpy = {};
    ib_cpy.dst = index_buffer;
    ib_cpy.dst_offset = 0;
    ib_cpy.src = upload_buffer;
    ib_cpy.src_offset = sizeof(CubeGeometry);
    ib_cpy.size = sizeof(CubeGeometry::g_Indices);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &ib_cpy);
    // wvp
    auto world = smath::make_transform(
        { 0.f, 0.f, 0.f },                        // translation
        2 * sakura::math::Vector3f::vector_one(), // scale
        sakura::math::Quaternion::identity()      // quat
    );
    // camera
    auto view = smath::look_at_matrix(
        { 0.f, 2.5f, 2.5f } /*eye*/,
        { 0.f, 0.f, 0.f } /*at*/);
    auto proj = smath::perspective_fov(
        3.1415926f / 2.f,
        (float)BACK_BUFFER_HEIGHT / (float)BACK_BUFFER_WIDTH,
        1.f, 1000.f);
    CubeGeometry::instance_data.world = world;
    CubeGeometry::instance_data.view_proj = smath::multiply(view, proj);
    {
        memcpy((char8_t*)upload_buffer->cpu_mapped_address + sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices),
            &CubeGeometry::instance_data, sizeof(CubeGeometry::InstanceData));
    }
    CGpuBufferToBufferTransfer istb_cpy = {};
    istb_cpy.dst = instance_buffer;
    istb_cpy.dst_offset = 0;
    istb_cpy.src = upload_buffer;
    istb_cpy.src_offset = sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices);
    istb_cpy.size = sizeof(CubeGeometry::instance_data);
    cgpu_cmd_transfer_buffer_to_buffer(cpy_cmd, &istb_cpy);
    CGpuBufferBarrier barriers[3];
    CGpuBufferBarrier& vb_barrier = barriers[0];
    vb_barrier.buffer = vertex_buffer;
    vb_barrier.src_state = RESOURCE_STATE_COPY_DEST;
    vb_barrier.dst_state = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    CGpuBufferBarrier& ib_barrier = barriers[1];
    ib_barrier.buffer = index_buffer;
    ib_barrier.src_state = RESOURCE_STATE_COPY_DEST;
    ib_barrier.dst_state = RESOURCE_STATE_INDEX_BUFFER;
    CGpuBufferBarrier& ist_barrier = barriers[2];
    ist_barrier.buffer = instance_buffer;
    ist_barrier.src_state = RESOURCE_STATE_COPY_DEST;
    ist_barrier.dst_state = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    CGpuResourceBarrierDescriptor barrier_desc = {};
    barrier_desc.buffer_barriers = barriers;
    barrier_desc.buffer_barriers_count = 3;
    cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc);
    cgpu_cmd_end(cpy_cmd);
    CGpuQueueSubmitDescriptor cpy_submit = {};
    cpy_submit.cmds = &cpy_cmd;
    cpy_submit.cmds_count = 1;
    cgpu_submit_queue(gfx_queue, &cpy_submit);
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_free_buffer(upload_buffer);
    cgpu_free_command_buffer(cpy_cmd);
    cgpu_free_command_pool(cmd_pool);
}

void create_gbuffer_render_pipeline()
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("rg-deferred/gbuffer_vs", &vs_bytes, &vs_length, backend);
    read_shader_bytes("rg-deferred/gbuffer_fs", &fs_bytes, &fs_length, backend);
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
    const char8_t* root_constant_name = "root_constants";
    CGpuRootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    rs_desc.root_constant_count = 1;
    rs_desc.root_constant_names = &root_constant_name;
    gbuffer_root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGpuVertexLayout vertex_layout = {};
    vertex_layout.attributes[0] = { "POSITION", PF_R32G32B32_SFLOAT, 0, 0, 0, INPUT_RATE_VERTEX, 1 };
    vertex_layout.attributes[1] = { "TEXCOORD", PF_R32G32_SFLOAT, 1, 1, 0, INPUT_RATE_VERTEX, 1 };
    vertex_layout.attributes[2] = { "NORMAL", PF_R8G8B8A8_SNORM, 2, 2, 0, INPUT_RATE_VERTEX, 1 };
    vertex_layout.attributes[3] = { "TANGENT", PF_R8G8B8A8_SNORM, 3, 3, 0, INPUT_RATE_VERTEX, 1 };
    vertex_layout.attributes[4] = { "MODEL", PF_R32G32B32A32_SFLOAT, 4, 4, 0, INPUT_RATE_INSTANCE, 4 };
    vertex_layout.attributes[5] = { "VIEWPROJ", PF_R32G32B32A32_SFLOAT, 5, 5, 0, INPUT_RATE_INSTANCE, 4 };
    vertex_layout.attribute_count = 6;
    CGpuRenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = gbuffer_root_sig;
    rp_desc.prim_topology = PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 2;
    rp_desc.color_formats = gbuffer_formats;
    gbuffer_pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(gbuffer_vs);
    cgpu_free_shader_library(gbuffer_fs);
}

void create_lighting_render_pipeline()
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("rg-deferred/screen_vs", &vs_bytes, &vs_length, backend);
    read_shader_bytes("rg-deferred/lighting_fs", &fs_bytes, &fs_length, backend);
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
    CGpuShaderLibraryId screen_vs = cgpu_create_shader_library(device, &vs_desc);
    CGpuShaderLibraryId lighting_fs = cgpu_create_shader_library(device, &ps_desc);
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
    lighting_root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGpuVertexLayout vertex_layout = {};
    vertex_layout.attribute_count = 0;
    CGpuRenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = lighting_root_sig;
    rp_desc.prim_topology = PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 1;
    auto backbuffer_format = (ECGpuFormat)swapchain->back_buffers[0]->format;
    rp_desc.color_formats = &backbuffer_format;
    lighting_pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(screen_vs);
    cgpu_free_shader_library(lighting_fs);
}

void create_render_pipeline()
{
    create_gbuffer_render_pipeline();
    create_lighting_render_pipeline();
}

void finalize()
{
    // Free cgpu objects
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    cgpu_free_buffer(index_buffer);
    cgpu_free_buffer(vertex_buffer);
    cgpu_free_buffer(instance_buffer);
    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
    cgpu_free_render_pipeline(gbuffer_pipeline);
    cgpu_free_root_signature(gbuffer_root_sig);
    cgpu_free_render_pipeline(lighting_pipeline);
    cgpu_free_root_signature(lighting_root_sig);
    cgpu_free_sampler(static_sampler);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

struct LightingPushConstants {
    int bFlipUVX = 0;
    int bFlipUVY = 0;
};
static LightingPushConstants lighting_data = {};

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;
    sdl_window = SDL_CreateWindow(gCGpuBackendNames[backend],
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window, &wmInfo);
    create_api_objects();
    create_render_pipeline();
    // initialize
    namespace render_graph = sakura::render_graph;
    auto graph = render_graph::RenderGraph::create(
        [=](render_graph::RenderGraphBuilder& builder) {
            builder.with_device(device)
                .with_gfx_queue(gfx_queue);
        });
    // loop
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (SDL_GetWindowID(sdl_window) == event.window.windowID)
            {
                if (!SDLEventHandler(&event, sdl_window))
                {
                    quit = true;
                }
            }
        }
        // acquire frame
        cgpu_wait_fences(&present_fence, 1);
        CGpuAcquireNextDescriptor acquire_desc = {};
        acquire_desc.fence = present_fence;
        backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
        // render graph setup & compile & exec
        CGpuTextureId to_import = swapchain->back_buffers[backbuffer_index];
        auto back_buffer = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name("backbuffer")
                    .import(to_import, RESOURCE_STATE_UNDEFINED)
                    .allow_render_target();
            });
        auto gbuffer_color = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name("gbuffer_color")
                    .extent(to_import->width, to_import->height)
                    .format(gbuffer_formats[0])
                    .allow_render_target();
            });
        auto gbuffer_normal = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name("gbuffer_normal")
                    .extent(to_import->width, to_import->height)
                    .format(gbuffer_formats[1])
                    .allow_render_target();
            });
        graph->add_render_pass(
            [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                builder.set_name("gbuffer_pass")
                    .set_pipeline(gbuffer_pipeline)
                    .write(0, gbuffer_color, LOAD_ACTION_CLEAR)
                    .write(1, gbuffer_normal, LOAD_ACTION_CLEAR);
            },
            [=](render_graph::RenderGraph& g, render_graph::RenderPassStack& stack) {
                cgpu_render_encoder_set_viewport(stack.encoder,
                    0.0f, 0.0f,
                    (float)to_import->width, (float)to_import->height,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, to_import->width, to_import->height);
                CGpuBufferId vertex_buffers[6] = {
                    vertex_buffer, vertex_buffer, vertex_buffer,
                    vertex_buffer, instance_buffer, instance_buffer
                };
                const uint32_t strides[6] = {
                    sizeof(sakura::math::Vector3f), sizeof(sakura::math::Vector2f),
                    sizeof(uint32_t), sizeof(uint32_t),
                    sizeof(CubeGeometry::InstanceData::world),
                    sizeof(CubeGeometry::InstanceData::view_proj)
                };
                const uint32_t offsets[6] = {
                    offsetof(CubeGeometry, g_Positions), offsetof(CubeGeometry, g_TexCoords),
                    offsetof(CubeGeometry, g_Normals), offsetof(CubeGeometry, g_Tangents),
                    offsetof(CubeGeometry::InstanceData, world),
                    offsetof(CubeGeometry::InstanceData, view_proj)
                };
                cgpu_render_encoder_bind_index_buffer(stack.encoder, index_buffer, sizeof(uint32_t), 0);
                cgpu_render_encoder_bind_vertex_buffers(stack.encoder, 6, vertex_buffers, strides, offsets);
                cgpu_render_encoder_draw_indexed_instanced(stack.encoder, 36, 0, 1, 0, 0);
            });
        graph->add_render_pass(
            [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                builder.set_name("light_pass")
                    .set_pipeline(lighting_pipeline)
                    .read(0, 0, gbuffer_color.read_mip(0, 1))
                    .read(0, 1, gbuffer_normal)
                    .write(0, back_buffer, LOAD_ACTION_CLEAR);
            },
            [=](render_graph::RenderGraph& g, render_graph::RenderPassStack& stack) {
                cgpu_render_encoder_set_viewport(stack.encoder,
                    0.0f, 0.0f,
                    (float)to_import->width, (float)to_import->height,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(stack.encoder, 0, 0, to_import->width, to_import->height);
                cgpu_render_encoder_push_constants(stack.encoder, lighting_pipeline->root_signature, "root_constants", &lighting_data);
                cgpu_render_encoder_draw(stack.encoder, 6, 0);
            });
        graph->add_present_pass(
            [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
                builder.set_name("present_pass")
                    .swapchain(swapchain, backbuffer_index)
                    .texture(back_buffer, true);
            });
        graph->compile();
        static uint32_t frame_index = 0;
        if (frame_index == 0)
            render_graph::RenderGraphViz::write_graphviz(*graph, "render_graph_deferred.gv");
        frame_index = graph->execute();
        // present
        cgpu_wait_queue_idle(gfx_queue);
        CGpuQueuePresentDescriptor present_desc = {};
        present_desc.index = backbuffer_index;
        present_desc.swapchain = swapchain;
        cgpu_queue_present(gfx_queue, &present_desc);
    }
    render_graph::RenderGraph::destroy(graph);
    // clean up
    finalize();
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}