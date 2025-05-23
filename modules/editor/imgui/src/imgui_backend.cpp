#include <SkrImGui/imgui_backend.hpp>
#include <SkrImGui/imgui_render_backend.hpp>
#include <SDL3/SDL.h>
#include <SkrCore/log.hpp>
#include "./private/imgui_impl_sdl3.h"

namespace skr
{
// ctor & dtor
ImGuiBackend::ImGuiBackend()
{
}
ImGuiBackend::~ImGuiBackend()
{
    if (is_created())
    {
        destroy();
    }
}

// imgui context
void ImGuiBackend::apply_context()
{
    SKR_ASSERT(is_created() && "please call create() before apply");
    SKR_ASSERT(!_renderer_backend.is_empty() && "please set render backend before apply");

    ImGui::SetCurrentContext(_context);
}
void ImGuiBackend::create(const ImGuiWindowCreateInfo& main_wnd_create_info, RCUnique<ImGuiRendererBackend> backend)
{
    SKR_ASSERT(!is_created() && "multi create context");

    _renderer_backend = std::move(backend);

    // create context
    _context = ImGui::CreateContext();

    // create main window
    _main_window.create(main_wnd_create_info);

    // init platform backend
    {
        ImGuiContext* cache = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(_context);
        ImGui_ImplSDL3_InitForOther(reinterpret_cast<SDL_Window*>(_main_window.payload()));
        ImGui::SetCurrentContext(cache);
    }

    // init render backend
    {
        _context->IO.BackendRendererUserData = _renderer_backend.get();
        _context->IO.BackendRendererName     = "Sakura ImGui Renderer";
        _context->IO.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
        _renderer_backend->setup_io(_context->IO);

        _context->PlatformIO.Renderer_CreateWindow = +[](ImGuiViewport* vp) {
            auto backend = static_cast<ImGuiRendererBackend*>(
                ImGui::GetCurrentContext()->IO.BackendRendererUserData
            );
            backend->create_window(vp);
        };
        _context->PlatformIO.Renderer_DestroyWindow = +[](ImGuiViewport* vp) {
            auto backend = static_cast<ImGuiRendererBackend*>(
                ImGui::GetCurrentContext()->IO.BackendRendererUserData
            );
            backend->destroy_window(vp);
        };
        _context->PlatformIO.Renderer_SetWindowSize = +[](ImGuiViewport* vp, ImVec2 size) {
            auto backend = static_cast<ImGuiRendererBackend*>(
                ImGui::GetCurrentContext()->IO.BackendRendererUserData
            );
            backend->resize_window(vp, size);
        };
        _context->PlatformIO.Renderer_RenderWindow = +[](ImGuiViewport* vp, void*) {
            auto backend = static_cast<ImGuiRendererBackend*>(
                ImGui::GetCurrentContext()->IO.BackendRendererUserData
            );
            backend->render_window(vp, nullptr);
        };
    }

    // init main window render data
    _renderer_backend->create_main_window(_context->Viewports[0]);
}
void ImGuiBackend::destroy()
{
    SKR_ASSERT(is_created() && "try destroy context before create");

    // wait rendering done
    _renderer_backend->wait_rendering_done();

    // destroy all textures
    {
        for (ImTextureData* tex : _context->PlatformIO.Textures)
        {
            if (tex->RefCount == 1)
            {
                _renderer_backend->destroy_texture(tex);
                tex->TexID  = 0;
                tex->Status = ImTextureStatus_Destroyed;
            }
        }
    }

    // destroy main window render data
    _renderer_backend->destroy_main_window(_context->Viewports[0]);

    // reset render backend
    {
        _context->IO.BackendRendererUserData = nullptr;
        _context->IO.BackendRendererName     = nullptr;

        _context->PlatformIO.Renderer_CreateWindow  = nullptr;
        _context->PlatformIO.Renderer_DestroyWindow = nullptr;
        _context->PlatformIO.Renderer_SetWindowSize = nullptr;
        _context->PlatformIO.Renderer_RenderWindow  = nullptr;
    }

    // destroy render backend
    _renderer_backend.reset();

    // destroy platform backend
    {
        ImGuiContext* cache = ImGui::GetCurrentContext();
        ImGui::SetCurrentContext(_context);
        ImGui_ImplSDL3_Shutdown();
        ImGui::SetCurrentContext(cache);
    }

    // destroy context
    ImGui::DestroyContext(_context);

    _context = nullptr;
}

// frame
void ImGuiBackend::pump_message()
{
    SKR_ASSERT(is_created() && "please create context before pump message");
    SDL_Event   e;
    SDL_Window* main_wnd = reinterpret_cast<SDL_Window*>(_main_window.payload());
    while (SDL_PollEvent(&e))
    {
        ImGui_ImplSDL3_ProcessEvent(&e);

        // quit message
        if (e.type == SDL_EVENT_QUIT)
        {
            _want_exit.trigger();
        }
        else if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                 e.window.windowID == SDL_GetWindowID(main_wnd))
        {
            _want_exit.trigger();
        }

        // resize message
        if (e.window.windowID == SDL_GetWindowID(main_wnd))
        {
            _want_resize.trigger_if(e.type == SDL_EVENT_WINDOW_RESIZED);
            _pixel_size_changed.trigger_if(e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED);
            _content_scale_changed.trigger_if(e.type == SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED);
        }
    }
}
void ImGuiBackend::begin_frame()
{
    SKR_ASSERT(is_created() && "please create context before begin frame");
    SKR_ASSERT(ImGui::GetCurrentContext() == _context && "context mismatch");

    apply_context();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    _renderer_backend->begin_frame();
}
void ImGuiBackend::end_frame()
{
    SKR_ASSERT(is_created() && "please create context before end frame");
    SKR_ASSERT(ImGui::GetCurrentContext() == _context && "context mismatch");

    ImGui::EndFrame();
    if (_context->IO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
    }
    _renderer_backend->end_frame();
}
void ImGuiBackend::render()
{
    SKR_ASSERT(is_created() && "please create context before render");
    SKR_ASSERT(ImGui::GetCurrentContext() == _context && "context mismatch");

    // update textures
    for (ImTextureData* tex : ImGui::GetPlatformIO().Textures)
    {
        switch (tex->Status)
        {
        case ImTextureStatus_WantCreate:
            _renderer_backend->create_texture(tex);
            SKR_ASSERT(tex->Status == ImTextureStatus_OK);
            break;
        case ImTextureStatus_WantUpdates:
            _renderer_backend->update_texture(tex);
            SKR_ASSERT(tex->Status == ImTextureStatus_OK);
            break;
        case ImTextureStatus_WantDestroy:
            if (tex->UnusedFrames >= _renderer_backend->backbuffer_count())
            {
                _renderer_backend->destroy_texture(tex);
                SKR_ASSERT(tex->Status == ImTextureStatus_Destroyed);
            }
            break;
        }
    }

    // resize main window
    if (_want_resize.comsume())
    {
        auto new_size = _main_window.size_client();
        _renderer_backend->resize_main_window(
            _context->Viewports[0],
            { (float)new_size.x, (float)new_size.y }
        );
    }

    // render main window
    ImGui::Render();
    _renderer_backend->render_main_window(
        _context->Viewports[0]
    );

    // render other viewports
    if (_context->IO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::RenderPlatformWindowsDefault();
    }
}

// imgui functional
void ImGuiBackend::enable_nav(bool enable)
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
void ImGuiBackend::enable_docking(bool enable)
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
void ImGuiBackend::enable_multi_viewport(bool enable)
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
void ImGuiBackend::enable_ini_file(bool enable)
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
void ImGuiBackend::enable_log_file(bool enable)
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
void ImGuiBackend::enable_transparent_docking(bool enable)
{
    SKR_ASSERT(is_created() && "please create context before set feature");
    _context->IO.ConfigDockingTransparentPayload = enable;
}
void ImGuiBackend::enable_always_tab_bar(bool enable)
{
    SKR_ASSERT(is_created() && "please create context before set feature");
    _context->IO.ConfigDockingAlwaysTabBar = enable;
}
void ImGuiBackend::enable_move_window_by_blank_area(bool enable)
{
    SKR_ASSERT(is_created() && "please create context before set feature");
    _context->IO.ConfigWindowsMoveFromTitleBarOnly = !enable;
}
void ImGuiBackend::enable_high_dpi(bool enable)
{
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
} // namespace skr