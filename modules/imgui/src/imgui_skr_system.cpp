#include "imgui/skr_imgui.config.h"
#include "utils/log.h"
#include "imgui/imgui.h"
#include "imgui/skr_imgui.h"
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace skr::imgui
{
SKR_IMGUI_API ImGuiContext*& imgui_context()
{
    static ImGuiContext* ctx = nullptr;
    return ctx;
}

void imgui_create_window(ImGuiViewport* viewport);
void imgui_destroy_window(ImGuiViewport* viewport);
void imgui_show_window(ImGuiViewport* viewport);
ImVec2 imgui_get_window_pos(ImGuiViewport* viewport);
void imgui_set_window_pos(ImGuiViewport* viewport, ImVec2 pos);
ImVec2 imgui_get_window_size(ImGuiViewport* viewport);
void imgui_set_window_size(ImGuiViewport* viewport, ImVec2 size);
void imgui_set_window_title(ImGuiViewport* viewport, const char* title);
void imgui_set_window_alpha(ImGuiViewport* viewport, float alpha);
void imgui_set_window_focus(ImGuiViewport* viewport);
bool imgui_get_window_focus(ImGuiViewport* viewport);
bool imgui_get_window_minimized(ImGuiViewport* viewport);
void imgui_render_window(ImGuiViewport* viewport, void*);
void imgui_swap_buffers(ImGuiViewport* viewport, void*);
} // namespace skr::imgui

static void imgui_update_mouse_and_buttons(SWindowHandle window)
{
    ImGuiIO& io = ImGui::GetIO();

    // [1]
    // Only when requested by io.WantSetMousePos: set OS mouse pos from Dear ImGui mouse pos.
    // (rarely used, mostly when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos)
    {
        skr_set_cursor_pos((uint32_t)io.MousePos.x, (uint32_t)io.MousePos.y);
    }

    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    io.MouseDown[0] = skr_mouse_key_down(EMouseKey::MOUSE_KEY_LB);
    io.MouseDown[1] = skr_mouse_key_down(EMouseKey::MOUSE_KEY_RB);
    io.MouseDown[2] = skr_mouse_key_down(EMouseKey::MOUSE_KEY_MB);

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
        // This is the position you can get with GetCursorPos(). In theory adding viewport->Pos is also the reverse operation of doing ScreenToClient().
        if (ImGui::FindViewportByPlatformHandle(window) != NULL)
        {
            int32_t pos_x, pos_y;
            skr_cursor_pos(&pos_x, &pos_y, CURSOR_COORDINATE_SCREEN);
            // int32_t window_x, window_y;
            // skr_window_get_position(window, &window_x, &window_y);
            // pos_x -= window_x;
            // pos_y -= window_y;
            io.AddMousePosEvent((float)pos_x, (float)pos_y);
        }
    }
    else
    {
        // Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window.)
        // This is the position you can get with GetCursorPos() + ScreenToClient() or from WM_MOUSEMOVE.
        if (skr_get_mouse_focused_window() == window)
        {
            int32_t pos_x, pos_y;
            skr_cursor_pos(&pos_x, &pos_y, CURSOR_COORDINATE_WINDOW);
            io.AddMousePosEvent((float)pos_x, (float)pos_y);
        }
    }
#ifdef _WIN32
    if (io.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport)
    {
        ImGuiID mouse_viewport_id = 0;
        POINT mouse_screen_pos;
        ::GetCursorPos(&mouse_screen_pos);
        if (HWND hovered_hwnd = ::WindowFromPoint(mouse_screen_pos))
        {
            ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
            for (auto i = 0; i < platform_io.Viewports.size(); i++)
            {
                if (platform_io.Viewports[i]->PlatformHandleRaw == hovered_hwnd)
                {
                    mouse_viewport_id = platform_io.Viewports[i]->ID;
                    break;
                }
            }
        }
        io.AddMouseViewportEvent(mouse_viewport_id);
        io.MouseHoveredViewport = mouse_viewport_id;
    }
#endif
}

void skr_imgui_new_frame(SWindowHandle window, float delta_time)
{
    ImGuiIO& io = ImGui::GetIO();

    // Configure backend flags
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
#ifdef _WIN32
    io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;
#endif

    int extent_w, extent_h;
    skr_window_get_extent(window, &extent_w, &extent_h);
    io.DisplaySize = { static_cast<float>(extent_w), static_cast<float>(extent_h) };

    // !! no Scalable Drawing Support.
    // TODO: Support.
    io.DisplayFramebufferScale = ImVec2(1, 1);

    // set delta time.
    io.DeltaTime = delta_time;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        // Register platform interface (will be coupled with a renderer interface)
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        platform_io.Platform_CreateWindow = &skr::imgui::imgui_create_window;
        platform_io.Platform_DestroyWindow = &skr::imgui::imgui_destroy_window;
        platform_io.Platform_ShowWindow = &skr::imgui::imgui_show_window;
        platform_io.Platform_SetWindowPos = &skr::imgui::imgui_set_window_pos;
        platform_io.Platform_GetWindowPos = &skr::imgui::imgui_get_window_pos;
        platform_io.Platform_SetWindowSize = &skr::imgui::imgui_set_window_size;
        platform_io.Platform_GetWindowSize = &skr::imgui::imgui_get_window_size;
        platform_io.Platform_SetWindowFocus = &skr::imgui::imgui_set_window_focus;
        platform_io.Platform_GetWindowFocus = &skr::imgui::imgui_get_window_focus;
        platform_io.Platform_GetWindowMinimized = &skr::imgui::imgui_get_window_minimized;
        platform_io.Platform_SetWindowTitle = &skr::imgui::imgui_set_window_title;
        platform_io.Platform_RenderWindow = &skr::imgui::imgui_render_window;
        platform_io.Platform_SwapBuffers = &skr::imgui::imgui_swap_buffers;

        ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        main_viewport->PlatformUserData = main_viewport; // hold this place
        main_viewport->PlatformHandle = window;
        main_viewport->PlatformHandleRaw = skr_window_get_native_handle(window);

        // update monitors
        eastl::vector<SMonitorHandle> skr_monitors;
        platform_io.Monitors.resize(0);
        uint32_t monitor_count = 0;
        skr_get_all_monitors(&monitor_count, nullptr);
        skr_monitors.resize(monitor_count);
        skr_get_all_monitors(&monitor_count, skr_monitors.data());
        platform_io.Monitors.resize(monitor_count);
        for (uint32_t i = 0; i < monitor_count; i++)
        {
            ImGuiPlatformMonitor& monitor = platform_io.Monitors[i];
            int32_t x, y, w, h;
            skr_monitor_get_position(skr_monitors[i], &x, &y);
            skr_monitor_get_extent(skr_monitors[i], &w, &h);
            monitor.MainPos = monitor.WorkPos = ImVec2((float)x, (float)y);
            monitor.MainSize = monitor.WorkSize = ImVec2((float)w, (float)h);

            float ddpi = 0.f;
            const bool success = skr_monitor_get_ddpi(skr_monitors[i], &ddpi, nullptr, nullptr);
            monitor.DpiScale = success ? (ddpi / 96.0f) : 1.f;
        }
    }
    // update inputs
    // ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    // for (int i = 0; i < platform_io.Viewports.Size; i++)
    {
        imgui_update_mouse_and_buttons(window);
    }

    ImGui::NewFrame();
}

void skr::imgui::imgui_create_window(ImGuiViewport* viewport)
{
    SWindowDescroptor desc = {};
    desc.flags = SKR_WINDOW_HIDDEN;
    desc.flags |= (viewport->Flags & ImGuiViewportFlags_NoDecoration) ? SKR_WINDOW_BOARDLESS : 0;
    desc.flags |= !(viewport->Flags & ImGuiViewportFlags_NoDecoration) ? 0 : SKR_WINDOW_RESIZABLE;
    desc.flags |= (viewport->Flags & ImGuiViewportFlags_TopMost) ? SKR_WINDOW_TOPMOST : 0;
    desc.width = (uint32_t)viewport->Size.x;
    desc.height = (uint32_t)viewport->Size.y;
    desc.posx = (uint32_t)viewport->Pos.x;
    desc.posy = (uint32_t)viewport->Pos.y;
    eastl::string title = "imgui-";
    title += eastl::to_string(viewport->ID);
    auto new_window = skr_create_window(title.c_str(), &desc);

    viewport->PlatformUserData = viewport;
    viewport->PlatformHandle = new_window;
    viewport->PlatformHandleRaw = skr_window_get_native_handle(new_window);
}

void skr::imgui::imgui_destroy_window(ImGuiViewport* viewport)
{
    skr_free_window((SWindowHandle)viewport->PlatformHandle);
    viewport->PlatformUserData = viewport->PlatformHandle = NULL;
}

void skr::imgui::imgui_show_window(ImGuiViewport* viewport)
{
#if defined(_WIN32)
    HWND hwnd = (HWND)viewport->PlatformHandleRaw;

    // SDL hack: Hide icon from task bar
    // Note: SDL 2.0.6+ has a SDL_WINDOW_SKIP_TASKBAR flag which is supported under Windows but the way it create the window breaks our seamless transition.
    if (viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon)
    {
        LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ex_style &= ~WS_EX_APPWINDOW;
        ex_style |= WS_EX_TOOLWINDOW;
        ::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
    }

    // SDL hack: SDL always activate/focus windows :/
    if (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)
    {
        ::ShowWindow(hwnd, SW_SHOWNA);
        return;
    }
#endif
    skr_show_window((SWindowHandle)viewport->PlatformHandle);
}

ImVec2 skr::imgui::imgui_get_window_pos(ImGuiViewport* viewport)
{
    SWindowHandle window = (SWindowHandle)viewport->PlatformHandle;
    int32_t x, y;
    skr_window_get_position(window, &x, &y);
    return ImVec2((float)x, (float)y);
}

void skr::imgui::imgui_set_window_pos(ImGuiViewport* viewport, ImVec2 pos)
{
    skr_window_set_position((SWindowHandle)viewport->PlatformHandle, (int32_t)pos.x, (int32_t)pos.y);
}

ImVec2 skr::imgui::imgui_get_window_size(ImGuiViewport* viewport)
{
    SWindowHandle window = (SWindowHandle)viewport->PlatformHandle;
    int32_t x, y;
    skr_window_get_extent(window, &x, &y);
    return ImVec2((float)x, (float)y);
}

void skr::imgui::imgui_set_window_size(ImGuiViewport* viewport, ImVec2 size)
{
    skr_window_set_extent((SWindowHandle)viewport->PlatformHandle, (int32_t)size.x, (int32_t)size.y);
}

void skr::imgui::imgui_set_window_title(ImGuiViewport* viewport, const char* title)
{
    skr_window_set_title((SWindowHandle)viewport->PlatformHandle, title);
}

void skr::imgui::imgui_set_window_alpha(ImGuiViewport* viewport, float alpha)
{
    SKR_LOG_INFO("imgui_set_window_alpha");

}

void skr::imgui::imgui_set_window_focus(ImGuiViewport* viewport)
{
    SKR_LOG_INFO("imgui_set_window_focus");
    
}

bool skr::imgui::imgui_get_window_focus(ImGuiViewport* viewport)
{
    return skr_window_is_focused((SWindowHandle)viewport->PlatformHandle);
}

bool skr::imgui::imgui_get_window_minimized(ImGuiViewport* viewport)
{
    return skr_window_is_minimized((SWindowHandle)viewport->PlatformHandle);
}

void skr::imgui::imgui_render_window(ImGuiViewport* viewport, void*)
{
    // nothing need to be done here...
}

void skr::imgui::imgui_swap_buffers(ImGuiViewport* viewport, void*)
{
    // nothing need to be done here...
}