#include <SkrImGui/imgui_app.hpp>
#include <SkrImGui/imgui_system_event_handler.hpp>
#include <SkrSystem/system_app.h>
#include <SkrSystem/window.h>
#include <SkrSystem/window_manager.h>
#include <SkrSystem/IME.h>
#include <SkrCore/log.hpp>
#include <SkrCore/memory/memory.h>
#include <SkrOS/filesystem.hpp>
#include <chrono>

namespace skr
{

// Clipboard functions
static const char* ImGui_ImplSkrSystem_GetClipboardText(ImGuiContext* ctx)
{
    ImGuiIO& io = ctx->IO;
    ImGuiApp* backend = static_cast<ImGuiApp*>(io.BackendPlatformUserData);
    if (backend)
    {
        auto ime = backend->get_ime();
        backend->_clipboard = ime->get_clipboard_text();
        return backend->_clipboard.c_str_raw();
    }
    return nullptr;
}

static void ImGui_ImplSkrSystem_SetClipboardText(ImGuiContext* ctx, const char* text)
{
    ImGuiIO& io = ctx->IO;
    ImGuiApp* backend = static_cast<ImGuiApp*>(io.BackendPlatformUserData);
    if (backend)
    {
        auto ime = backend->get_ime();
        ime->set_clipboard_text(skr::String((const char8_t*)text));
    }
}

static float ImGui_ImplSkrSystem_GetWindowDpiScale(ImGuiViewport* viewpoer)
{
    const auto window = (SystemWindow*)viewpoer->PlatformHandle;
    const auto ratio = window->get_pixel_ratio();
    return ratio;
}

// ctor & dtor
ImGuiApp::ImGuiApp(const SystemWindowCreateInfo& main_wnd_create_info, SRenderDeviceId render_device, skr::render_graph::RenderGraphBuilder& builder)
    : skr::RenderApp(render_device, builder), _main_window_info(main_wnd_create_info)
{

}

ImGuiApp::~ImGuiApp()
{
    
}

bool ImGuiApp::initialize(const char* backend)
{
    if (!RenderApp::initialize(backend))
        return false;

    SKR_ASSERT(!is_created() && "multi create context");

    // Create context
    _context = ImGui::CreateContext();

    // create main window
    _main_window_index = open_window(_main_window_info);
    _main_window = get_window(_main_window_index);

    // Create event handler and register with event queue
    _event_handler = SkrNew<ImGuiSystemEventHandler>(this);
    event_queue->add_handler(_event_handler);

    // Setup ImGui platform backend
    {
        ImGuiContext* cache = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(_context);

        // Initialize ImGui IO for SkrSystem
        ImGuiIO& io = _context->IO;
        io.BackendPlatformUserData = this; // Store backend pointer for callbacks
        io.BackendPlatformName = "imgui_impl_skr_system";
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;      // We can honor GetMouseCursor() values (optional)
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;       // We can honor io.WantSetMousePos requests (optional, rarely used)
        io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports; // We can create multi-viewports on the Platform side (optional)

        // Setup platform functions
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        platform_io.Platform_GetWindowDpiScale = ImGui_ImplSkrSystem_GetWindowDpiScale;
        platform_io.Platform_SetClipboardTextFn = ImGui_ImplSkrSystem_SetClipboardText;
        platform_io.Platform_GetClipboardTextFn = ImGui_ImplSkrSystem_GetClipboardText;
        platform_io.Platform_SetImeDataFn = [](ImGuiContext* ctx, ImGuiViewport* viewport, ImGuiPlatformImeData* data) {
            ImGuiIO& io = ctx->IO;
            ImGuiApp* backend = static_cast<ImGuiApp*>(io.BackendPlatformUserData);
            if (backend)
            {
                if (data->WantVisible)
                {
                    // Set input area for IME window positioning
                    IMETextInputArea area;
                    area.position = {
                        static_cast<int32_t>(data->InputPos.x - viewport->Pos.x),
                        static_cast<int32_t>(data->InputPos.y - viewport->Pos.y)
                    };
                    area.size = {
                        200, // Default width
                        static_cast<uint32_t>(data->InputLineHeight)
                    };
                    area.cursor_height = static_cast<uint32_t>(data->InputLineHeight);

                    backend->get_ime()->set_text_input_area(area);
                }

                // Note: We don't start/stop IME here because UpdateIMEState() already handles it
                // based on WantTextInput. This callback is mainly for positioning the IME window.
            }
        };

        ImGui::SetCurrentContext(cache);
    }

    // Setup IME if available
    if (get_ime())
    {
        SetupIMECallbacks();
    }

    // Initialize render backend
    {
        _context->IO.BackendRendererUserData = this;
        _context->IO.BackendRendererName = "Sakura ImGui Renderer";
        _context->IO.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
        _context->IO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        _context->IO.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
    }

    // Setup main viewport platform handles
    // Our mouse update function expect PlatformHandle to be filled for the main viewport
    auto* main_viewport = _context->Viewports[0];
    if (_main_window)
    {
        // Store window ID as PlatformHandle (similar to SDL3's approach)
        // This allows us to find viewport by window later
        main_viewport->PlatformHandle = _main_window;
        main_viewport->PlatformHandleRaw = _main_window->get_native_view();
    }

    // Show the window
    if (_main_window)
    {
        _main_window->show();
    }

    // Initialize time tracking
    last_frame_time_ = std::chrono::steady_clock::now();

    // create render pipeline
    create_pipeline();

    return true;
}

void ImGuiApp::shutdown()
{
    SKR_ASSERT(is_created() && "try destroy context before create");

    // Wait rendering done
    auto gfx_queue = _render_device->get_gfx_queue();
    cgpu_wait_queue_idle(gfx_queue);

    // Stop IME if active
    if (ime && ime->is_text_input_active())
    {
        ime->stop_text_input();
    }

    // Unregister event handler
    if (_event_handler)
    {
        event_queue->remove_handler(_event_handler);
        SkrDelete(_event_handler);
        _event_handler = nullptr;
    }

    // Destroy all textures
    {
        for (ImTextureData* tex : _context->PlatformIO.Textures)
        {
            if (tex->RefCount == 1)
            {
                destroy_texture(tex);
                tex->TexID = 0;
                tex->Status = ImTextureStatus_Destroyed;
            }
        }
    }

    // Reset platform backend callbacks and user data
    {
        _context->IO.BackendPlatformUserData = nullptr;
        _context->IO.BackendPlatformName = nullptr;
    }

    // Reset render backend callbacks and user data
    {
        _context->IO.BackendRendererUserData = nullptr;
        _context->IO.BackendRendererName = nullptr;
    }

    // Destroy main window through WindowManager
    if (_main_window)
    {
        close_window(_main_window_index);
        _main_window = nullptr;
    }

    // Destroy context
    ImGui::DestroyContext(_context);
    _context = nullptr;

    RenderApp::shutdown();

    // destroy render pipeline & root signature
    cgpu_free_render_pipeline(_render_pipeline);
    cgpu_free_root_signature(_root_signature);
    cgpu_free_sampler(_static_sampler);
}

void ImGuiApp::render_imgui()
{
    SKR_ASSERT(is_created() && "please create context before end frame");
    SKR_ASSERT(ImGui::GetCurrentContext() == _context && "context mismatch");

    ImGui::EndFrame();
    if (_context->IO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
    }

    for (ImTextureData* tex : ImGui::GetPlatformIO().Textures)
    {
        switch (tex->Status)
        {
        case ImTextureStatus_WantCreate:
            create_texture(tex);
            SKR_ASSERT(tex->Status == ImTextureStatus_OK);
            break;
        case ImTextureStatus_WantUpdates:
            update_texture(tex);
            SKR_ASSERT(tex->Status == ImTextureStatus_OK);
            break;
        case ImTextureStatus_WantDestroy:
            if (tex->UnusedFrames >= 10)
            {
                destroy_texture(tex);
                SKR_ASSERT(tex->Status == ImTextureStatus_Destroyed);
            }
            break;
        default:
            break;
        }
    }

    // Render main window
    ImGui::Render();

    // run pass
    add_render_pass(ImGui::GetMainViewport(), _graph, _root_signature, _render_pipeline);

    // Render other viewports
    if (_context->IO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::RenderPlatformWindowsDefault();
    }
}

// Legacy compatibility - pump_message delegates to process_events
void ImGuiApp::pump_message()
{
    SKR_ASSERT(is_created() && "please create context before begin frame");
    SKR_ASSERT(ImGui::GetCurrentContext() == _context && "context mismatch");

    event_queue->pump_messages();
    ImGui::SetCurrentContext(_context);

    // Update ImGui display size (every frame to accommodate for window resizing)
    auto _main_window = get_main_window();
    if (_main_window)
    {
        auto window_size = _main_window->get_size();

        // Handle minimized window
        int w = window_size.x;
        int h = window_size.y;
        if (_main_window->is_minimized())
            w = h = 0;

        _context->IO.DisplaySize = ImVec2((float)w, (float)h);

        // Update framebuffer scale
        if (w > 0 && h > 0)
        {
            auto pixel_ratio = _main_window->get_pixel_ratio();
            _context->IO.DisplayFramebufferScale = ImVec2(pixel_ratio, pixel_ratio);
        }
    }

    // Update delta time (use member variable instead of static)
    auto current_time = std::chrono::steady_clock::now();
    float delta_time = std::chrono::duration<float>(current_time - last_frame_time_).count();
    _context->IO.DeltaTime = delta_time > 0.0f ? delta_time : (1.0f / 60.0f);

    // Update IME state based on ImGui needs
    UpdateIMEState();

    // Update mouse cursor
    UpdateMouseCursor();

    ImGui::NewFrame();
}

// Helper to update mouse cursor
void ImGuiApp::UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        // TODO: Implement cursor hiding when SystemWindow supports it
    }
    else
    {
        // Show OS mouse cursor and set shape
        // TODO: Implement cursor shape setting when SystemWindow supports it
        // Future implementation would map ImGuiMouseCursor to system cursors:
        // ImGuiMouseCursor_Arrow -> default
        // ImGuiMouseCursor_TextInput -> text/ibeam
        // ImGuiMouseCursor_ResizeAll -> move/all directions
        // ImGuiMouseCursor_ResizeNS -> resize vertical
        // ImGuiMouseCursor_ResizeEW -> resize horizontal
        // ImGuiMouseCursor_ResizeNESW -> resize diagonal 1
        // ImGuiMouseCursor_ResizeNWSE -> resize diagonal 2
        // ImGuiMouseCursor_Hand -> hand/pointer
        // ImGuiMouseCursor_Wait -> wait/hourglass
        // ImGuiMouseCursor_Progress -> progress/app starting
        // ImGuiMouseCursor_NotAllowed -> not allowed/no
    }
}

// imgui functional
void ImGuiApp::enable_nav(bool enable)
{
    SKR_ASSERT(is_created() && "please create context before set feature");
    if (enable)
    {
        _context->IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        _context->IO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    }
    else
    {
        _context->IO.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
        _context->IO.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
    }
}

void ImGuiApp::enable_docking(bool enable)
{
    SKR_ASSERT(is_created() && "please create context before set feature");
    if (enable)
    {
        _context->IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }
    else
    {
        _context->IO.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
    }
}

void ImGuiApp::enable_multi_viewport(bool enable)
{
    SKR_ASSERT(is_created() && "please create context before set feature");
    if (enable)
    {
        _context->IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    }
    else
    {
        _context->IO.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
    }
}

void ImGuiApp::enable_ini_file(bool enable)
{
    SKR_ASSERT(is_created() && "please create context before set feature");
    if (enable)
    {
        _context->IO.IniFilename = "imgui.ini";
    }
    else
    {
        _context->IO.IniFilename = nullptr;
    }
}

void ImGuiApp::enable_log_file(bool enable)
{
    SKR_ASSERT(is_created() && "please create context before set feature");
    if (enable)
    {
        _context->IO.LogFilename = "imgui.log";
    }
    else
    {
        _context->IO.LogFilename = nullptr;
    }
}

void ImGuiApp::enable_transparent_docking(bool enable)
{
    SKR_ASSERT(is_created() && "please create context before set feature");
    _context->IO.ConfigDockingTransparentPayload = enable;
}

void ImGuiApp::enable_always_tab_bar(bool enable)
{
    SKR_ASSERT(is_created() && "please create context before set feature");
    _context->IO.ConfigDockingAlwaysTabBar = enable;
}

void ImGuiApp::enable_move_window_by_blank_area(bool enable)
{
    SKR_ASSERT(is_created() && "please create context before set feature");
    _context->IO.ConfigWindowsMoveFromTitleBarOnly = !enable;
}

void ImGuiApp::enable_high_dpi(bool enable)
{
    SKR_ASSERT(is_created() && "please create context before set feature");
    if (enable)
    {
        _context->IO.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
        _context->IO.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
    }
    else
    {
        _context->IO.ConfigFlags &= ~ImGuiConfigFlags_DpiEnableScaleFonts;
        _context->IO.ConfigFlags &= ~ImGuiConfigFlags_DpiEnableScaleViewports;
    }
}

void ImGuiApp::set_load_action(ECGPULoadAction action)
{
    _load_action = action;
}

void ImGuiApp::SetupIMECallbacks()
{
    SKR_LOG_DEBUG(u8"Setting up IME callbacks");

    IMEEventCallbacks callbacks;

    // Most important callback: receive committed text
    callbacks.on_text_input = [this](const skr::String& text) {
        SKR_LOG_DEBUG(u8"IME text input received: %s (length: %d)", text.c_str(), text.size());

        if (_context && !text.is_empty())
        {
            // Ensure we're in the correct ImGui context
            ImGuiContext* prev_ctx = ImGui::GetCurrentContext();
            ImGui::SetCurrentContext(_context);

            // Add text to ImGui - convert from char8_t* to char*
            _context->IO.AddInputCharactersUTF8(reinterpret_cast<const char*>(text.c_str()));

            SKR_LOG_DEBUG(u8"Added text to ImGui IO");

            ImGui::SetCurrentContext(prev_ctx);
        }
    };

    // Composition text callback (optional, for showing text being typed)
    callbacks.on_composition_update = [this](const IMECompositionInfo& info) {
        // ImGui doesn't currently support showing composition text directly
        // But we could add custom rendering here if needed
        // For now, just log for debugging
        if (!info.text.is_empty())
        {
            SKR_LOG_TRACE(u8"IME composition: %s (cursor: %d)", info.text.c_str(), info.cursor_pos);
        }
    };

    // Candidates update callback (optional)
    callbacks.on_candidates_update = [this](const IMECandidateInfo& info) {
        // Could create custom candidate window
        // For now, let system IME show default candidate window
        SKR_LOG_TRACE(u8"IME candidates updated: %d candidates", info.total_candidates);
    };

    ime->set_event_callbacks(callbacks);

    SKR_LOG_DEBUG(u8"IME callbacks set successfully");
}

void ImGuiApp::UpdateIMEState()
{
    if (!ime || !_main_window)
        return;

    // Simple state sync: automatically manage IME based on WantTextInput
    bool imgui_wants_text = _context->IO.WantTextInput;

    // Use our own state tracking instead of querying IME every frame
    if (imgui_wants_text && !_ime_active_state)
    {
        // ImGui wants text input but we haven't activated IME yet
        ime->start_text_input(_main_window);
        _ime_active_state = true;

        SKR_LOG_DEBUG(u8"Starting IME text input (WantTextInput=true)");
    }
    else if (!imgui_wants_text && _ime_active_state)
    {
        // ImGui doesn't want text input but we have IME active
        ime->stop_text_input();
        _ime_active_state = false;
        SKR_LOG_DEBUG(u8"Stopping IME text input (WantTextInput=false)");
    }
}

inline static Vector<uint8_t> _read_shader_bytes(
    String virtual_path,
    ECGPUBackend backend)
{
    Vector<uint8_t> result;

    // combine path
    skr::Path shader_path{u8"./../resources/shaders/"};
    shader_path /= virtual_path.c_str();
    skr::String ext;
    switch (backend)
    {
    case CGPU_BACKEND_VULKAN:
        ext = u8".spv";
        break;
    case CGPU_BACKEND_D3D12:
    case CGPU_BACKEND_XBOX_D3D12:
        ext = u8".dxil";
        break;
    default:
        break;
    }
    auto path_str = shader_path.string();
    path_str.append(ext);
    shader_path = skr::Path{path_str};

    // read file
    if (skr::fs::File::exists(shader_path))
    {
        if (!skr::fs::File::read_all_bytes(shader_path, result))
        {
            SKR_LOG_ERROR(u8"failed to read shader file: %s", shader_path.string().c_str());
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"shader file not found: %s", shader_path.string().c_str());
    }

    return result;
}

void ImGuiApp::create_pipeline()
{
    auto _gfx_queue = _render_device->get_gfx_queue();
    // load shaders
    CGPUShaderEntryDescriptor ppl_shaders[2] = {};
    auto vs_bytes = _read_shader_bytes(
        u8"imgui.vs",
        _gfx_queue->device->adapter->instance->backend);
    auto ps_bytes = _read_shader_bytes(
        u8"imgui.fs",
        _gfx_queue->device->adapter->instance->backend);

    // create lib
    CGPUShaderLibraryDescriptor vs_desc{};
    vs_desc.name = SKR_UTF8("imgui_vertex_shader");
    vs_desc.code = reinterpret_cast<uint32_t*>(vs_bytes.data());
    vs_desc.code_size = vs_bytes.size();
    CGPUShaderLibraryDescriptor ps_desc{};
    ps_desc.name = SKR_UTF8("imgui_fragment_shader");
    ps_desc.code = reinterpret_cast<uint32_t*>(ps_bytes.data());
    ps_desc.code_size = ps_bytes.size();
    auto vs_lib = cgpu_create_shader_library(
        _gfx_queue->device,
        &vs_desc);
    auto fs_lib = cgpu_create_shader_library(
        _gfx_queue->device,
        &ps_desc);

    // fill desc
    ppl_shaders[0].library = vs_lib;
    ppl_shaders[0].entry = SKR_UTF8("vs");
    ppl_shaders[1].library = fs_lib;
    ppl_shaders[1].entry = SKR_UTF8("fs");

    // load static sampler
    CGPUSamplerDescriptor sampler_desc{};
    sampler_desc.address_u = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_v = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_w = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode = CGPU_MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.compare_func = CGPU_CMP_NEVER;
    _static_sampler = cgpu_create_sampler(
        _gfx_queue->device,
        &sampler_desc);

    // create root signature
    {
        const char8_t* push_constant_name = u8"push_constants";
        const char8_t* sampler_name = u8"sampler0";
        CGPURootSignatureDescriptor rs_desc{};
        rs_desc.shaders = ppl_shaders;
        rs_desc.shader_count = 2;
        rs_desc.push_constant_names = &push_constant_name;
        rs_desc.push_constant_count = 1;
        rs_desc.static_sampler_names = &sampler_name;
        rs_desc.static_sampler_count = 1;
        rs_desc.static_samplers = &_static_sampler;
        rs_desc.name = u8"ImGuiDraw-RootSignature";
        _root_signature = cgpu_create_root_signature(
            _gfx_queue->device,
            &rs_desc);
    }

    // create pipeline
    {
        CGPUVertexLayout vertex_layout{};
        vertex_layout.attribute_count = 3;
        vertex_layout.attributes[0] = {
            u8"pos",
            1,
            CGPU_FORMAT_R32G32_SFLOAT,
            0,
            0,
            sizeof(float) * 2,
            CGPU_INPUT_RATE_VERTEX
        };
        vertex_layout.attributes[1] = {
            u8"uv",
            1,
            CGPU_FORMAT_R32G32_SFLOAT,
            0,
            sizeof(float) * 2,
            sizeof(float) * 2,
            CGPU_INPUT_RATE_VERTEX
        };
        vertex_layout.attributes[2] = {
            u8"color",
            1,
            CGPU_FORMAT_R8G8B8A8_UNORM,
            0,
            sizeof(float) * 4,
            sizeof(uint32_t),
            CGPU_INPUT_RATE_VERTEX
        };

        CGPURasterizerStateDescriptor rs_state{};
        rs_state.cull_mode = CGPU_CULL_MODE_NONE;
        rs_state.fill_mode = CGPU_FILL_MODE_SOLID;
        rs_state.front_face = CGPU_FRONT_FACE_CW;
        rs_state.slope_scaled_depth_bias = 0.f;
        rs_state.enable_depth_clamp = false;
        rs_state.enable_scissor = true;
        rs_state.enable_multi_sample = false;
        rs_state.depth_bias = 0;

        CGPUBlendStateDescriptor blend_state{};
        blend_state.blend_modes[0] = CGPU_BLEND_MODE_ADD;
        blend_state.src_factors[0] = CGPU_BLEND_CONST_SRC_ALPHA;
        blend_state.dst_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
        blend_state.blend_alpha_modes[0] = CGPU_BLEND_MODE_ADD;
        blend_state.src_alpha_factors[0] = CGPU_BLEND_CONST_ONE;
        blend_state.dst_alpha_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
        blend_state.masks[0] = CGPU_COLOR_MASK_ALL;
        blend_state.independent_blend = false;

        CGPURenderPipelineDescriptor rp_desc{};
        rp_desc.root_signature = _root_signature;
        rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
        rp_desc.vertex_layout = &vertex_layout;
        rp_desc.vertex_shader = &ppl_shaders[0];
        rp_desc.fragment_shader = &ppl_shaders[1];
        rp_desc.render_target_count = 1;
        rp_desc.rasterizer_state = &rs_state;
        rp_desc.blend_state = &blend_state;
        rp_desc.color_formats = &_backbuffer_format;

        _render_pipeline = cgpu_create_render_pipeline(
            _gfx_queue->device,
            &rp_desc);
    }

    // cleanup shader library
    cgpu_free_shader_library(ppl_shaders[0].library);
    cgpu_free_shader_library(ppl_shaders[1].library);
}

void ImGuiApp::add_render_pass(
    ImGuiViewport* vp,
    render_graph::RenderGraph* render_graph,
    CGPURootSignatureId root_sig,
    CGPURenderPipelineId render_pipeline)
{
    SkrZoneScopedN("RenderIMGUI");
    namespace rg = skr::render_graph;

    // get data
    auto draw_data = vp->DrawData;
    auto load_action = _load_action;
    if (draw_data->TotalVtxCount == 0) { return; }
    uint32_t vertex_size = draw_data->TotalVtxCount * (uint32_t)sizeof(ImDrawVert);
    uint32_t index_size = draw_data->TotalIdxCount * (uint32_t)sizeof(ImDrawIdx);
    auto swapchain = get_swapchain(_main_window);
    auto bbindex = backbuffer_index(_main_window);
    auto backbuffer = render_graph->get_imported(swapchain->back_buffers[bbindex]);

    // use CVV
    bool useCVV = true;
#if SKR_PLAT_MACOSX
    useCVV = false;
#endif

    // create or resize vb/ib
    auto vertex_buffer_handle = render_graph->create_buffer(
        [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
            SkrZoneScopedN("ConstructVBHandle");

            String name = skr::format(u8"imgui_vertices-{}", draw_data->OwnerViewport->ID);
            builder.set_name(name.c_str())
                .size(vertex_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT : CGPU_BUFFER_FLAG_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        });
    auto index_buffer_handle = render_graph->create_buffer(
        [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
            SkrZoneScopedN("ConstructIBHandle");

            String name = skr::format(u8"imgui_indices-{}", draw_data->OwnerViewport->ID);
            builder.set_name(name.c_str())
                .size(index_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT : CGPU_BUFFER_FLAG_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_index_buffer();
        });

    // upload vb/ib
    if (!useCVV)
    {
        auto upload_buffer_handle = render_graph->create_buffer(
            [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
                SkrZoneScopedN("ConstructUploadPass");

                String name = skr::format(u8"imgui_upload-{}", draw_data->OwnerViewport->ID);
                builder.set_name(name.c_str())
                    .size(index_size + vertex_size)
                    .with_tags(kRenderGraphDefaultResourceTag)
                    .as_upload_buffer();
            });
        render_graph->add_copy_pass(
            [=](rg::RenderGraph& g, rg::CopyPassBuilder& builder) {
                SkrZoneScopedN("ConstructCopyPass");

                String name = skr::format(u8"imgui_copy-{}", draw_data->OwnerViewport->ID);
                builder.set_name(name.c_str())
                    .buffer_to_buffer(upload_buffer_handle.range(0, vertex_size), vertex_buffer_handle.range(0, vertex_size))
                    .buffer_to_buffer(upload_buffer_handle.range(vertex_size, vertex_size + index_size), index_buffer_handle.range(0, index_size));
            },
            [upload_buffer_handle, draw_data](rg::RenderGraph& g, rg::CopyPassContext& context) {
                auto upload_buffer = context.resolve(upload_buffer_handle);
                ImDrawVert* vtx_dst = (ImDrawVert*)upload_buffer->info->cpu_mapped_address;
                ImDrawIdx* idx_dst = (ImDrawIdx*)(vtx_dst + draw_data->TotalVtxCount);
                for (int n = 0; n < draw_data->CmdListsCount; n++)
                {
                    const ImDrawList* cmd_list = draw_data->CmdLists[n];
                    memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                    memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                    vtx_dst += cmd_list->VtxBuffer.Size;
                    idx_dst += cmd_list->IdxBuffer.Size;
                }
            });
    }

    // cbuffer
    auto constant_buffer = render_graph->create_buffer(
        [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
            SkrZoneScopedN("ConstructCBHandle");

            String name = skr::format(u8"imgui_cbuffer-{}", draw_data->OwnerViewport->ID);
            builder.set_name(name.c_str())
                .size(sizeof(float) * 4 * 4)
                .memory_usage(CGPU_MEM_USAGE_CPU_TO_GPU)
                .with_flags(CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT)
                .prefer_on_device()
                .as_constant_buffer();
        });

    // import textures
    rg::TextureHandle font_texture;
    for (auto tex : ImGui::GetCurrentContext()->PlatformIO.Textures)
    {
        if (tex->Status == ImTextureStatus_OK)
        {
            font_texture = render_graph->create_texture(
                [=](rg::RenderGraph& g, rg::TextureBuilder& builder) {
                    SkrZoneScopedN("ConstructTextureHandle");
                    auto tex_data = (ImGuiRendererBackendRGTextureData*)tex->BackendUserData;
                    String name = skr::format(u8"imgui_font-{}", tex->UniqueID);
                    builder.set_name((const char8_t*)name.c_str())
                        .import(tex_data->texture, CGPU_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                });
            break;
        }
    }

    // render passes
    render_graph->add_render_pass(
        [=](rg::RenderGraph& g, rg::RenderPassBuilder& builder) {
            SkrZoneScopedN("ConstructRenderPass");

            String name = skr::format(u8"imgui_render-{}", draw_data->OwnerViewport->ID);
            builder.set_name(name.c_str())
                .set_pipeline(render_pipeline)
                .read(u8"Constants", constant_buffer.range(0, sizeof(float) * 4 * 4))
                .use_buffer(vertex_buffer_handle, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
                .use_buffer(index_buffer_handle, CGPU_RESOURCE_STATE_INDEX_BUFFER)
                .read(u8"texture0", font_texture)
                .write(0, backbuffer, load_action);
        },
        [backbuffer, useCVV, draw_data, constant_buffer, index_buffer_handle, vertex_buffer_handle](rg::RenderGraph& g, rg::RenderPassContext& context) {
            SkrZoneScopedN("ImGuiPass");

            // get info
            const auto target_desc = g.resolve_descriptor(backbuffer);
            SKR_ASSERT(target_desc && "ImGui render target not found!");

            // upload cbuffer
            {
                float L = draw_data->DisplayPos.x;
                float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
                float T = draw_data->DisplayPos.y;
                float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
                float mvp[4][4] = {
                    { 2.0f / (R - L), 0.0f, 0.0f, 0.0f },
                    { 0.0f, 2.0f / (T - B), 0.0f, 0.0f },
                    { 0.0f, 0.0f, 0.5f, 0.0f },
                    { (R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f },
                };
                auto buf = context.resolve(constant_buffer);
                memcpy(buf->info->cpu_mapped_address, mvp, sizeof(mvp));
            }

            // set viewport
            cgpu_render_encoder_set_viewport(context.encoder, 0.0f, 0.0f, (float)target_desc->width, (float)target_desc->height, 0.f, 1.f);

            // upload IB/VB
            auto resolved_ib = context.resolve(index_buffer_handle);
            auto resolved_vb = context.resolve(vertex_buffer_handle);
            if (useCVV)
            {
                // upload
                ImDrawVert* vtx_dst = (ImDrawVert*)resolved_vb->info->cpu_mapped_address;
                ImDrawIdx* idx_dst = (ImDrawIdx*)resolved_ib->info->cpu_mapped_address;
                for (int n = 0; n < draw_data->CmdListsCount; n++)
                {
                    const ImDrawList* cmd_list = draw_data->CmdLists[n];
                    memcpy(
                        vtx_dst,
                        cmd_list->VtxBuffer.Data,
                        cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                    memcpy(
                        idx_dst,
                        cmd_list->IdxBuffer.Data,
                        cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                    vtx_dst += cmd_list->VtxBuffer.Size;
                    idx_dst += cmd_list->IdxBuffer.Size;
                }
            }

            // draw commands
            int global_vtx_offset = 0;
            int global_idx_offset = 0;
            const ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
            const ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
                {
                    const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                    // draw
                    if (pcmd->UserCallback != NULL)
                    {
                    }
                    else
                    {
                        // Project scissor/clipping rectangles into framebuffer space
                        ImVec4 clip_rect;
                        clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                        clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                        clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                        clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;
                        if (clip_rect.x < 0.0f) clip_rect.x = 0.0f;
                        if (clip_rect.y < 0.0f) clip_rect.y = 0.0f;
                        cgpu_render_encoder_set_scissor(
                            context.encoder,
                            (uint32_t)clip_rect.x,
                            (uint32_t)clip_rect.y,
                            (uint32_t)(clip_rect.z - clip_rect.x),
                            (uint32_t)(clip_rect.w - clip_rect.y));
                        cgpu_render_encoder_bind_index_buffer(
                            context.encoder,
                            resolved_ib,
                            sizeof(uint16_t),
                            0);
                        const uint32_t vert_stride = sizeof(ImDrawVert);
                        cgpu_render_encoder_bind_vertex_buffers(
                            context.encoder,
                            1,
                            &resolved_vb,
                            &vert_stride,
                            NULL);
                        cgpu_render_encoder_draw_indexed(
                            context.encoder,
                            pcmd->ElemCount,
                            pcmd->IdxOffset + global_idx_offset,
                            pcmd->VtxOffset + global_vtx_offset);
                    }
                }
                global_idx_offset += cmd_list->IdxBuffer.Size;
                global_vtx_offset += cmd_list->VtxBuffer.Size;
            }
        });
}

void ImGuiApp::destroy_texture(ImTextureData* tex_data)
{
    // destroy user data
    auto user_data = (ImGuiRendererBackendRGTextureData*)tex_data->BackendUserData;
    cgpu_free_texture(user_data->texture);
    cgpu_free_texture_view(user_data->srv);
    SkrDelete(user_data);

    // reset data
    tex_data->TexID = 0;
    tex_data->BackendUserData = nullptr;
    tex_data->Status = ImTextureStatus_Destroyed;
}

void ImGuiApp::create_texture(ImTextureData* tex_data)
{
    // create user data
    auto _gfx_queue = _render_device->get_gfx_queue();
    auto user_data = SkrNew<ImGuiRendererBackendRGTextureData>();
    tex_data->BackendUserData = user_data;

    // create texture
    CGPUTextureDescriptor tex_desc = {};
    tex_desc.name = u8"imgui_font";
    tex_desc.width = static_cast<uint32_t>(tex_data->Width);
    tex_desc.height = static_cast<uint32_t>(tex_data->Height);
    tex_desc.depth = 1;
    tex_desc.usages = CGPU_TEXTURE_USAGE_SHADER_READ;
    tex_desc.array_size = 1;
    tex_desc.flags = CGPU_TEXTURE_FLAG_NONE;
    tex_desc.mip_levels = 1;
    tex_desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
    tex_desc.start_state = CGPU_RESOURCE_STATE_COPY_DEST;
    tex_desc.owner_queue = _gfx_queue;
    user_data->texture = cgpu_create_texture(_gfx_queue->device, &tex_desc);

    // create texture view
    CGPUTextureViewDescriptor view_desc{};
    view_desc.texture = user_data->texture;
    view_desc.base_array_layer = 0;
    view_desc.array_layer_count = 1;
    view_desc.base_mip_level = 0;
    view_desc.mip_level_count = 1;
    view_desc.format = user_data->texture->info->format;
    view_desc.aspects = CGPU_TEXTURE_VIEW_ASPECTS_COLOR;
    view_desc.view_usages = CGPU_TEXTURE_VIEW_USAGE_SRV;
    view_desc.dims = CGPU_TEXTURE_DIMENSION_2D;
    user_data->srv = cgpu_create_texture_view(_gfx_queue->device, &view_desc);

    tex_data->TexID = reinterpret_cast<ImTextureID>(user_data->srv);

    // upload data
    user_data->first_update = true;
    update_texture(tex_data);
}

void ImGuiApp::update_texture(ImTextureData* tex_data)
{
    // get user data
    auto _gfx_queue = _render_device->get_gfx_queue();
    auto user_data = (ImGuiRendererBackendRGTextureData*)tex_data->BackendUserData;

    // create command buffer
    CGPUCommandPoolDescriptor cmd_pool_desc = {};
    CGPUCommandBufferDescriptor cmd_desc = {
        .name = u8"CommandBuffer-ImGuiUpdateTexture"
    };
    auto cpy_cmd_pool = cgpu_create_command_pool(
        _gfx_queue,
        &cmd_pool_desc);
    auto cpy_cmd = cgpu_create_command_buffer(
        cpy_cmd_pool,
        &cmd_desc);

    // create upload buffer
    // TODO. use updata rect
    CGPUBufferDescriptor upload_buffer_desc{};
    upload_buffer_desc.name = u8"IMGUI_FontUploadBuffer";
    upload_buffer_desc.flags = CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT;
    upload_buffer_desc.usages = CGPU_BUFFER_USAGE_NONE;
    upload_buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size = tex_data->GetSizeInBytes();
    CGPUBufferId tex_upload_buffer = cgpu_create_buffer(_gfx_queue->device, &upload_buffer_desc);

    // copy data
    memcpy(
        tex_upload_buffer->info->cpu_mapped_address,
        tex_data->Pixels,
        tex_data->GetSizeInBytes());

    // combine commands
    cgpu_cmd_begin(cpy_cmd);
    {
        if (!user_data->first_update)
        {
            // srv -> copy_dst
            CGPUTextureBarrier cpy_dst_barrier{};
            cpy_dst_barrier.texture = user_data->texture;
            cpy_dst_barrier.src_state = CGPU_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            cpy_dst_barrier.dst_state = CGPU_RESOURCE_STATE_COPY_DEST;
            {
                CGPUResourceBarrierDescriptor barrier_desc = {};
                barrier_desc.texture_barriers = &cpy_dst_barrier;
                barrier_desc.texture_barriers_count = 1;
                cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc);
            }
        }

        // copy
        CGPUBufferToTextureTransfer b2t{};
        b2t.src = tex_upload_buffer;
        b2t.src_offset = 0;
        b2t.dst = user_data->texture;
        b2t.dst_subresource.mip_level = 0;
        b2t.dst_subresource.base_array_layer = 0;
        b2t.dst_subresource.layer_count = 1;
        cgpu_cmd_transfer_buffer_to_texture(cpy_cmd, &b2t);

        // copy_dst -> srv
        CGPUTextureBarrier srv_barrier{};
        srv_barrier.texture = user_data->texture;
        srv_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
        srv_barrier.dst_state = CGPU_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        {
            CGPUResourceBarrierDescriptor barrier_desc = {};
            barrier_desc.texture_barriers = &srv_barrier;
            barrier_desc.texture_barriers_count = 1;
            cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc);
        }
        user_data->first_update = false;
    }
    cgpu_cmd_end(cpy_cmd);

    // submit commands
    CGPUQueueSubmitDescriptor cpy_submit{};
    cpy_submit.cmds = &cpy_cmd;
    cpy_submit.cmds_count = 1;
    cgpu_submit_queue(_gfx_queue, &cpy_submit);

    // wait for completion
    // TODO. use frame resource
    cgpu_wait_queue_idle(_gfx_queue);
    cgpu_free_command_buffer(cpy_cmd);
    cgpu_free_command_pool(cpy_cmd_pool);
    cgpu_free_buffer(tex_upload_buffer);

    tex_data->Status = ImTextureStatus_OK;
}

} // namespace skr