#include "../../common/utils.h"
#include "platform/thread.h"
#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include "module/module.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrImGui/skr_imgui.h"
#include "SkrImGui/skr_imgui_rg.h"
#ifdef SKR_OS_WINDOWS
    #include <shellscalingapi.h>
#endif

class SVMemCCModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char** argv) override;
    virtual int main_module_exec(int argc, char** argv) override;
    virtual void on_unload() override;

    void create_swapchain();
    void create_api_objects();
    void initialize_imgui();
    void finalize();

    void imgui_ui();

    bool DPIAware = false;
    SWindowHandle window;
    // Vulkan's memory heap is far more accurate than the one provided by D3D12
    ECGPUBackend backend = CGPU_BACKEND_VULKAN;
    // ECGPUBackend backend = CGPU_BACKEND_D3D12;

    CGPUInstanceId instance = nullptr;
    CGPUAdapterId adapter = nullptr;
    CGPUDeviceId device = nullptr;
    CGPUQueueId gfx_queue = nullptr;
    CGPUSamplerId static_sampler = nullptr;

    CGPUSurfaceId surface = nullptr;
    CGPUSwapChainId swapchain = nullptr;
    uint32_t backbuffer_index;
    CGPUFenceId present_fence = nullptr;

    float vbuffer_size = 0.001f;
    float sbuffer_size = 0.001f;
    eastl::vector<CGPUBufferId> buffers;

    skr::render_graph::RenderGraph* graph = nullptr;
};

IMPLEMENT_DYNAMIC_MODULE(SVMemCCModule, VMemController);

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
    ImGui::SameLine();
    ImGui::Text("Usable VMem: %.3f MB", total_mb - used_mb);
    
    uint64_t total_shared_bytes = 0;
    uint64_t used_shared_bytes = 0;
    cgpu_query_shared_memory_info(device, &total_shared_bytes, &used_shared_bytes);
    const auto total_shared_mb = (float)total_shared_bytes / 1024.f / 1024.f;
    const auto used_shared_mb = (float)used_shared_bytes / 1024.f / 1024.f;
    ImGui::Text("Used SMem: %.3f MB", used_shared_mb);
    ImGui::SameLine();
    ImGui::Text("Usable SVMem: %.3f MB", total_shared_mb - used_shared_mb);

    ImGui::SliderFloat(u8"##vbuffer", &vbuffer_size, 0.001f, total_mb - used_mb, "%.3f MB"); // in MB
    ImGui::SameLine();
    if (ImGui::Button(u8"AllocateVideoMemory"))
    {
        auto buf_desc = make_zeroed<CGPUBufferDescriptor>();
        buf_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
        buf_desc.descriptors = CGPU_RESOURCE_TYPE_VERTEX_BUFFER;
        buf_desc.memory_usage = CGPU_MEM_USAGE_GPU_ONLY;
        buf_desc.size = (uint64_t)(vbuffer_size * 1024 * 1024);
        buf_desc.name = "VideoMemory";
        auto new_buf = cgpu_create_buffer(device, &buf_desc);
        buffers.emplace_back(new_buf);
    }

    ImGui::SliderFloat(u8"##sbuffer", &sbuffer_size, 0.001f, total_shared_mb - used_shared_mb, "%.3f MB"); // in MB
    ImGui::SameLine();
    if (ImGui::Button(u8"AllocateSharedMemory"))
    {
        auto buf_desc = make_zeroed<CGPUBufferDescriptor>();
        buf_desc.flags = CGPU_BCF_OWN_MEMORY_BIT;
        buf_desc.descriptors = CGPU_RESOURCE_TYPE_NONE;
        buf_desc.memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU;
        buf_desc.size = (uint64_t)(sbuffer_size * 1024 * 1024);
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
        clipper.Begin((int)buffers.size());
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

#include "runtime_module.h"

int SVMemCCModule::main_module_exec(int argc, char** argv)
{
    DPIAware = skr_runtime_is_dpi_aware();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) return -1;
    SWindowDescroptor window_desc = {};
    window_desc.flags = SKR_WINDOW_CENTERED | SKR_WINDOW_RESIZABLE;
    window_desc.height = 1024;
    window_desc.width = 1024;
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
                if (event.type == SDL_WINDOWEVENT)
                {
                    Uint8 window_event = event.window.event;
                    if (window_event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        cgpu_wait_fences(&present_fence, 1);
                        create_swapchain();
                    }
                }
                if (!SDLEventHandler(&event, sdl_window))
                {
                    quit = true;
                }
            }
            
            if (event.type == SDL_WINDOWEVENT)
            {
                Uint8 window_event = event.window.event;
                if (window_event == SDL_WINDOWEVENT_CLOSE || window_event == SDL_WINDOWEVENT_MOVED || window_event == SDL_WINDOWEVENT_RESIZED)
                if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)SDL_GetWindowFromID(event.window.windowID)))
                {
                    if (window_event == SDL_WINDOWEVENT_CLOSE)
                        viewport->PlatformRequestClose = true;
                    if (window_event == SDL_WINDOWEVENT_MOVED)
                        viewport->PlatformRequestMove = true;
                    if (window_event == SDL_WINDOWEVENT_RESIZED)
                        viewport->PlatformRequestResize = true;
                }
            }

            if (event.type == SDL_QUIT)
            {
                quit = true;
                break;
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
            if (frame_index >= RG_MAX_FRAME_IN_FLIGHT * 10)
                graph->collect_garbage(frame_index - RG_MAX_FRAME_IN_FLIGHT * 10);
        }
        {
            CGPUQueuePresentDescriptor present_desc = {};
            present_desc.index = backbuffer_index;
            present_desc.swapchain = swapchain;
            cgpu_queue_present(gfx_queue, &present_desc);
            render_graph_imgui_present_sub_viewports();
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
    create_swapchain();
}

void SVMemCCModule::create_swapchain()
{
    if (swapchain)
    {
        cgpu_free_swapchain(swapchain);
    }
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
    ImGuiIO& io = ImGui::GetIO(); 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
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
            ImGui::GetIO().FontGlobalScale = 1.f / dpi_scaling;
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
