#include "../../cgpu/common/utils.h"
#include "platform/vfs.h"
#include "platform/thread.h"
#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include "render_graph/frontend/render_graph.hpp"
#include "imgui/skr_imgui.h"
#include "imgui/skr_imgui_rg.h"
#include "imgui/imgui.h"
#ifdef SKR_OS_WINDOWS
    #include <shellscalingapi.h>
#endif

class SVMemCCModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual int main_module_exec(int argc, char** argv) override;
    virtual void on_unload() override;

    void create_api_objects();
    void initialize_imgui();
    void finalize();

    void imgui_ui();

    bool DPIAware = false;
    SWindowHandle window;
    ECGPUBackend backend = CGPU_BACKEND_D3D12;

    CGPUInstanceId instance;
    CGPUAdapterId adapter;
    CGPUDeviceId device;
    CGPUQueueId gfx_queue;
    CGPUSamplerId static_sampler;

    CGPUSurfaceId surface;
    CGPUSwapChainId swapchain;
    uint32_t backbuffer_index;
    CGPUFenceId present_fence;

    float buffer_size = 0.001f;
    eastl::vector<CGPUBufferId> buffers;

    skr::render_graph::RenderGraph* graph = nullptr;
};

IMPLEMENT_DYNAMIC_MODULE(SVMemCCModule, VMemController);
SKR_MODULE_METADATA(u8R"(
{
    "api" : "0.1.0",
    "name" : "VMemController",
    "prettyname" : "VMemController",
    "version" : "0.0.1",
    "linking" : "shared",
    "dependencies" : [
        {"name":"SkrImGui", "version":"0.1.0"},
        {"name":"SkrRenderGraph", "version":"0.1.0"}
    ],
    "author" : "",
    "url" : "",
    "license" : "",
    "copyright" : ""
}
)",
VMemController)

void SVMemCCModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("vmem controller loaded!");
    for (auto i = 0; i < argc; i++)
    {
        if (::strcmp(argv[i], "--vulkan") == 0)
        {
            backend = CGPU_BACKEND_VULKAN;
        }
        else if (::strcmp(argv[i], "--d3d12") == 0)
        {
            backend = CGPU_BACKEND_D3D12;
        }
    }
}

void SVMemCCModule::imgui_ui()
{
    ImGui::Begin(u8"VideoMemoryController");
    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    auto adapter_detail = cgpu_query_adapter_detail(adapter);
    // information
    {
        ImGui::Text("Card: %s", adapter_detail->vendor_preset.gpu_name);
        ImGui::Text("Driver: %d", adapter_detail->vendor_preset.driver_version);
    }
    // allocation
    uint64_t total_bytes = 0;
    uint64_t used_bytes = 0;
    cgpu_query_video_memory_info(device, &total_bytes, &used_bytes);
    const auto total_mb = (float)total_bytes / 1024.f / 1024.f;
    const auto used_mb = (float)used_bytes / 1024.f / 1024.f;
    ImGui::Text("Used VMem: %.3f MB", used_mb);
    ImGui::Text("Usable VMem: %.3f MB", total_mb - used_mb);
    ImGui::SliderFloat("Size", &buffer_size, 0.001f, total_mb - used_mb, "%.3f MB"); // in MB
    if (ImGui::Button(u8"AllocateVideoMemory"))
    {
        auto buf_desc = make_zeroed<CGPUBufferDescriptor>();
        buf_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
        buf_desc.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
        buf_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
        buf_desc.size = (uint64_t)(buffer_size * 1024 * 1024);
        buf_desc.name = "VideoMemory";
        auto new_buf = cgpu_create_buffer(device, &buf_desc);
        buffers.emplace_back(new_buf);
    }
    ImGui::SameLine();
    if (ImGui::Button(u8"AllocateSharedMemory"))
    {
        auto buf_desc = make_zeroed<CGPUBufferDescriptor>();
        buf_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
        buf_desc.descriptors = CGPU_RESOURCE_TYPE_NONE;
        buf_desc.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
        buf_desc.size = (uint64_t)(buffer_size * 1024 * 1024);
        buf_desc.name = "SharedMemory";
        auto new_buf = cgpu_create_buffer(device, &buf_desc);
        buffers.emplace_back(new_buf);
    }
    // table
    const ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody
        | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("table_sorting", 4, flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 10), 0.0f))
    {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, 0);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
        ImGui::TableSetupColumn("Size(MB)", ImGuiTableColumnFlags_WidthFixed, 0.0f, 2);
        ImGui::TableSetupColumn("Free", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, 3);
        ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
        ImGui::TableHeadersRow();

        // Demonstrate using clipper for large vertical lists
        ImGuiListClipper clipper;
        clipper.Begin(buffers.size());
        while (clipper.Step())
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
            {
                // Display a data item
                auto buffer = buffers[row_n];
                ImGui::PushID(row_n);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%04d", row_n);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted((buffer->memory_usage == CGPU_MEM_USAGE_GPU_ONLY) ? "VideoMemory" : "SharedMemory");
                ImGui::TableNextColumn();
                ImGui::Text("%.3f", (float)buffer->size / 1024.f / 1024.f);
                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Delete"))
                {
                    cgpu_free_buffer(buffer);
                    buffers[row_n] = nullptr;
                }
                ImGui::PopID();
            }
        ImGui::EndTable();
    }
    ImGui::End();
}

int SVMemCCModule::main_module_exec(int argc, char** argv)
{
#ifdef SKR_OS_WINDOWS
    ::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    DPIAware = true;
#endif
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;
    SWindowDescroptor window_desc = {};
    window_desc.centered = true;
    window_desc.resizable = true;
    window_desc.height = 512;
    window_desc.width = 512;
    window = skr_create_window(gCGPUBackendNames[backend], &window_desc);
    // initialize api objects
    create_api_objects();
    // initialize render graph
    namespace render_graph = skr::render_graph;
    graph = render_graph::RenderGraph::create(
    [=](skr::render_graph::RenderGraphBuilder& builder) {
        builder.with_device(device)
        .with_gfx_queue(gfx_queue)
        .enable_memory_aliasing();
    });
    // initialize imgui
    initialize_imgui();
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        auto sdl_window = (SDL_Window*)window;
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
        static uint64_t frame_index = 0;
        auto& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(
            (float)swapchain->back_buffers[0]->width,
            (float)swapchain->back_buffers[0]->height);
        skr_imgui_new_frame(window, 1.f / 60.f);
        imgui_ui();
        buffers.erase(eastl::remove(buffers.begin(), buffers.end(), nullptr), buffers.end());
        {
            // acquire frame
            cgpu_wait_fences(&present_fence, 1);
            CGPUAcquireNextDescriptor acquire_desc = {};
            acquire_desc.fence = present_fence;
            backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
        }
        CGPUTextureId native_backbuffer = swapchain->back_buffers[backbuffer_index];
        // render graph setup & compile & exec
        auto back_buffer = graph->create_texture(
        [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
            builder.set_name("backbuffer")
            .import(native_backbuffer, CGPU_RESOURCE_STATE_UNDEFINED)
            .allow_render_target();
        });
        render_graph_imgui_add_render_pass(graph, back_buffer, CGPU_LOAD_ACTION_CLEAR);
        graph->add_present_pass(
            [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
                builder.set_name("present_pass")
                .swapchain(swapchain, backbuffer_index)
                .texture(back_buffer, true);
            });

        graph->compile();
        frame_index = graph->execute();
        {
            if (frame_index >= RG_MAX_FRAME_IN_FLIGHT)
                graph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT);
        }
        {
            CGPUQueuePresentDescriptor present_desc = {};
            present_desc.index = backbuffer_index;
            present_desc.swapchain = swapchain;
            cgpu_queue_present(gfx_queue, &present_desc);
            // Avoid too much CPU Usage
            skr_thread_sleep(16);
        }
    }
    return 0;
}

void SVMemCCModule::on_unload()
{
    SKR_LOG_INFO("vmem controller unloaded!");
    cgpu_wait_queue_idle(gfx_queue);
    skr::render_graph::RenderGraph::destroy(graph);
    render_graph_imgui_finalize();
    // clean up
    finalize();
    skr_free_window(window);
    SDL_Quit();
}

void SVMemCCModule::create_api_objects()
{
    // Create instance
    CGPUInstanceDescriptor instance_desc = {};
    instance_desc.backend = backend;
    instance_desc.enable_debug_layer = true;
    instance_desc.enable_gpu_based_validation = false;
    instance_desc.enable_set_name = true;
    instance = cgpu_create_instance(&instance_desc);

    // Filter adapters
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, CGPU_NULLPTR, &adapters_count);
    CGPUAdapterId adapters[64];
    cgpu_enum_adapters(instance, adapters, &adapters_count);
    adapter = adapters[0];

    // Create device
    CGPUQueueGroupDescriptor queue_group_desc = {};
    queue_group_desc.queue_type = CGPU_QUEUE_TYPE_GRAPHICS;
    queue_group_desc.queue_count = 1;
    CGPUDeviceDescriptor device_desc = {};
    device_desc.queue_groups = &queue_group_desc;
    device_desc.queue_group_count = 1;
    device = cgpu_create_device(adapter, &device_desc);
    gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);
    present_fence = cgpu_create_fence(device);
    // Sampler
    CGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_v = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_w = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode = CGPU_MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.compare_func = CGPU_CMP_NEVER;
    static_sampler = cgpu_create_sampler(device, &sampler_desc);

    // Create swapchain
    surface = cgpu_surface_from_native_view(device, skr_window_get_native_view(window));
    int32_t wwidth, wheight;
    skr_window_get_extent(window, &wwidth, &wheight);
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.present_queues = &gfx_queue;
    chain_desc.present_queues_count = 1;
    chain_desc.width = wwidth;
    chain_desc.height = wheight;
    chain_desc.surface = surface;
    chain_desc.imageCount = 2;
    chain_desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
    chain_desc.enable_vsync = false;
    swapchain = cgpu_create_swapchain(device, &chain_desc);
}

void SVMemCCModule::initialize_imgui()
{
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    {
        auto& style = ImGui::GetStyle();
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        const char* font_path = "./../resources/font/SourceSansPro-Regular.ttf";
        uint32_t *font_bytes, font_length;
        read_bytes(font_path, (char**)&font_bytes, &font_length);
        float dpi_scaling = 1.f;
        if (!DPIAware)
        {
            float ddpi;
            SDL_GetDisplayDPI(0, &ddpi, NULL, NULL);
            dpi_scaling = ddpi / OS_DPI;
            // scale back
            style.ScaleAllSizes(1.f / dpi_scaling);
            ImGui::GetIO().FontGlobalScale = 0.5f;
        }
        else
        {
            float ddpi;
            SDL_GetDisplayDPI(0, &ddpi, NULL, NULL);
            dpi_scaling = ddpi / OS_DPI;
            // scale back
            style.ScaleAllSizes(dpi_scaling);
        }
        ImFontConfig cfg = {};
        cfg.SizePixels = 16.f * dpi_scaling;
        cfg.OversampleH = cfg.OversampleV = 1;
        cfg.PixelSnapH = true;
        ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_bytes,
        font_length, cfg.SizePixels, &cfg);
        ImGui::GetIO().Fonts->Build();
        free(font_bytes);
    }
    uint32_t *im_vs_bytes, im_vs_length;
    read_shader_bytes("imgui_vertex", &im_vs_bytes, &im_vs_length,
    device->adapter->instance->backend);
    uint32_t *im_fs_bytes, im_fs_length;
    read_shader_bytes("imgui_fragment", &im_fs_bytes, &im_fs_length,
    device->adapter->instance->backend);
    CGPUShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = "imgui_vertex_shader";
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.code = im_vs_bytes;
    vs_desc.code_size = im_vs_length;
    CGPUShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = "imgui_fragment_shader";
    fs_desc.stage = CGPU_SHADER_STAGE_FRAG;
    fs_desc.code = im_fs_bytes;
    fs_desc.code_size = im_fs_length;
    CGPUShaderLibraryId imgui_vs = cgpu_create_shader_library(device, &vs_desc);
    CGPUShaderLibraryId imgui_fs = cgpu_create_shader_library(device, &fs_desc);
    free(im_vs_bytes);
    free(im_fs_bytes);
    RenderGraphImGuiDescriptor imgui_graph_desc = {};
    imgui_graph_desc.render_graph = graph;
    imgui_graph_desc.backbuffer_format = (ECGPUFormat)swapchain->back_buffers[backbuffer_index]->format;
    imgui_graph_desc.vs.library = imgui_vs;
    imgui_graph_desc.vs.stage = CGPU_SHADER_STAGE_VERT;
    imgui_graph_desc.vs.entry = "main";
    imgui_graph_desc.ps.library = imgui_fs;
    imgui_graph_desc.ps.stage = CGPU_SHADER_STAGE_FRAG;
    imgui_graph_desc.ps.entry = "main";
    imgui_graph_desc.queue = gfx_queue;
    imgui_graph_desc.static_sampler = static_sampler;
    render_graph_imgui_initialize(&imgui_graph_desc);
    cgpu_free_shader_library(imgui_vs);
    cgpu_free_shader_library(imgui_fs);
}

void SVMemCCModule::finalize()
{
    // Free cgpu objects
    cgpu_wait_fences(&present_fence, 1);
    cgpu_free_fence(present_fence);
    for (auto buffer : buffers)
    {
        cgpu_free_buffer(buffer);
    }
    cgpu_free_swapchain(swapchain);
    cgpu_free_surface(device, surface);
    cgpu_free_sampler(static_sampler);
    cgpu_free_queue(gfx_queue);
    cgpu_free_device(device);
    cgpu_free_instance(instance);
}
