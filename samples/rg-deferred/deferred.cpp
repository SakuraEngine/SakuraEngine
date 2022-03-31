#include "cube_geometry.h"
#include "../common/utils.h"
#include "render_graph/frontend/render_graph.hpp"

thread_local SDL_Window* sdl_window;
thread_local SDL_SysWMinfo wmInfo;
thread_local CGpuSurfaceId surface;
thread_local CGpuSwapChainId swapchain;
thread_local uint32_t backbuffer_index;
thread_local CGpuFenceId present_fence;

thread_local ECGpuBackend backend = CGPU_BACKEND_VULKAN;
thread_local CGpuInstanceId instance;
thread_local CGpuAdapterId adapter;
thread_local CGpuDeviceId device;
thread_local CGpuQueueId gfx_queue;

thread_local CGpuBufferId index_buffer;
thread_local CGpuBufferId vertex_buffer;

thread_local CGpuRootSignatureId root_sig;
thread_local CGpuRenderPipelineId pipeline;

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

    // upload
    swapchain = cgpu_create_swapchain(device, &chain_desc);
    CGpuBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name = "UploadBuffer";
    upload_buffer_desc.flags = BCF_OWN_MEMORY_BIT | BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors = RT_NONE;
    upload_buffer_desc.memory_usage = MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size = sizeof(CubeGeometry) + sizeof(CubeGeometry::g_Indices);
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
    CGpuBufferBarrier barriers[2];
    CGpuBufferBarrier& vb_barrier = barriers[0];
    vb_barrier.buffer = vertex_buffer;
    vb_barrier.src_state = RESOURCE_STATE_COPY_DEST;
    vb_barrier.dst_state = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    CGpuBufferBarrier& ib_barrier = barriers[1];
    ib_barrier.buffer = index_buffer;
    ib_barrier.src_state = RESOURCE_STATE_COPY_DEST;
    ib_barrier.dst_state = RESOURCE_STATE_INDEX_BUFFER;
    CGpuResourceBarrierDescriptor barrier_desc = {};
    barrier_desc.buffer_barriers = barriers;
    barrier_desc.buffer_barriers_count = 2;
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

void create_render_pipeline()
{
    uint32_t *vs_bytes, vs_length;
    uint32_t *fs_bytes, fs_length;
    read_shader_bytes("rg-deferred/vertex_shader", &vs_bytes, &vs_length, backend);
    read_shader_bytes("rg-deferred/fragment_shader", &fs_bytes, &fs_length, backend);
    CGpuShaderLibraryDescriptor vs_desc = {};
    vs_desc.stage = SHADER_STAGE_VERT;
    vs_desc.name = "VertexShaderLibrary";
    vs_desc.code = vs_bytes;
    vs_desc.code_size = vs_length;
    CGpuShaderLibraryDescriptor ps_desc = {};
    ps_desc.name = "FragmentShaderLibrary";
    ps_desc.stage = SHADER_STAGE_FRAG;
    ps_desc.code = fs_bytes;
    ps_desc.code_size = fs_length;
    CGpuShaderLibraryId vertex_shader = cgpu_create_shader_library(device, &vs_desc);
    CGpuShaderLibraryId fragment_shader = cgpu_create_shader_library(device, &ps_desc);
    free(vs_bytes);
    free(fs_bytes);
    CGpuPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0].stage = SHADER_STAGE_VERT;
    ppl_shaders[0].entry = "main";
    ppl_shaders[0].library = vertex_shader;
    ppl_shaders[1].stage = SHADER_STAGE_FRAG;
    ppl_shaders[1].entry = "main";
    ppl_shaders[1].library = fragment_shader;
    CGpuRootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    root_sig = cgpu_create_root_signature(device, &rs_desc);
    CGpuVertexLayout vertex_layout = {};
    vertex_layout.attributes[0] = { "POSITION", PF_R32G32B32_SFLOAT, 0, 0, 0, INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", PF_R32G32_SFLOAT, 1, 1, 0, INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "NORMAL", PF_R8G8B8A8_SNORM, 2, 2, 0, INPUT_RATE_VERTEX };
    vertex_layout.attributes[3] = { "TANGENT", PF_R8G8B8A8_SNORM, 3, 3, 0, INPUT_RATE_VERTEX };
    vertex_layout.attribute_count = 4;
    CGpuRenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 1;
    auto backend_format = (ECGpuFormat)swapchain->back_buffers[0]->format;
    rp_desc.color_formats = &backend_format;
    pipeline = cgpu_create_render_pipeline(device, &rp_desc);
    cgpu_free_shader_library(vertex_shader);
    cgpu_free_shader_library(fragment_shader);
}

void finalize()
{
    SDL_DestroyWindow(sdl_window);
    // Free cgpu objects
    cgpu_wait_queue_idle(gfx_queue);
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    cgpu_free_buffer(index_buffer);
    cgpu_free_buffer(vertex_buffer);
    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
    cgpu_free_render_pipeline(pipeline);
    cgpu_free_root_signature(root_sig);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}

namespace smath = sakura::math;
typedef struct PushConstants {
    sakura::math::float4x4 world;
    sakura::math::float4x4 view_proj;
} PushConstants;

static PushConstants data = {};

int main(int argc, char* argv[])
{
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
    data.world = smath::transpose(world);
    data.view_proj = smath::transpose(smath::multiply(view, proj));

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
    while (sdl_window)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (SDL_GetWindowID(sdl_window) == event.window.windowID && !SDLEventHandler(&event, sdl_window))
            {
                sdl_window = CGPU_NULLPTR;
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
                    .import(to_import)
                    .allow_render_target();
            });
        graph->add_render_pass(
            [=](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
                builder.set_name("color_pass")
                    .write(0, back_buffer.load_action(LOAD_ACTION_CLEAR));
            },
            [=](render_graph::RenderGraph& g, CGpuRenderPassEncoderId encoder) {
                cgpu_render_encoder_set_viewport(encoder,
                    0.0f, 0.0f,
                    (float)to_import->width, (float)to_import->height,
                    0.f, 1.f);
                cgpu_render_encoder_set_scissor(encoder, 0, 0, to_import->width, to_import->height);
                cgpu_render_encoder_bind_pipeline(encoder, pipeline);
                cgpu_render_encoder_bind_index_buffer(encoder, index_buffer, sizeof(uint32_t), 0);
                CGpuBufferId vertex_buffers[4] = { vertex_buffer, vertex_buffer, vertex_buffer, vertex_buffer };
                const uint32_t strides[4] = {
                    sizeof(CubeGeometry::g_Positions), sizeof(CubeGeometry::g_TexCoords),
                    sizeof(CubeGeometry::g_Normals), sizeof(CubeGeometry::g_Tangents)
                };
                const uint32_t offsets[4] = {
                    offsetof(CubeGeometry, g_Positions), offsetof(CubeGeometry, g_TexCoords),
                    offsetof(CubeGeometry, g_Normals), offsetof(CubeGeometry, g_Tangents)
                };
                cgpu_render_encoder_bind_vertex_buffers(encoder, 4, vertex_buffers, strides, offsets);
                cgpu_render_encoder_push_constants(encoder, pipeline->root_signature, "root_constants", &data);
                cgpu_render_encoder_draw_indexed(encoder, 36, 0, 0);
            });
        graph->add_present_pass(
            [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
                builder.set_name("present")
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
    SDL_Quit();
    return 0;
}