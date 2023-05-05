#include "SkrImGui/skr_imgui.config.h"
#include <EASTL/optional.h>
#include <EASTL/vector.h>
#include <EASTL/fixed_vector.h>
#include <containers/string.hpp>
#include "utils/log.h"
#include "platform/input.h"
#include "platform/system.h"

#include "SkrInput/input.h"

#include "SkrImGui/skr_imgui.h"
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
#include <windows.h>
#endif

#include "tracy/Tracy.hpp"

namespace skr::imgui
{
RUNTIME_EXPORT ImGuiContext* GImGuiContext = nullptr;

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

int32_t KeyCodeTranslator(EKeyCode keycode);
} // namespace skr::imgui

static void imgui_update_mouse_and_buttons(SWindowHandle window)
{
    ImGuiIO& io = ImGui::GetIO();

    // Update cursor position
    if (io.WantSetMousePos)
    {
        skr_set_cursor_pos((uint32_t)io.MousePos.x, (uint32_t)io.MousePos.y);
    }

    // Update mouse button states
    if (auto inputInst = skr::input::Input::GetInstance())
    {
        skr::input::InputLayer* input_layer = nullptr;
        skr::input::InputReading* input_reading = nullptr;
        auto res = inputInst->GetCurrentReading(
            skr::input::InputKindMouse, nullptr, &input_layer, &input_reading);
        skr::input::InputMouseState mouse_state = {};
        if (res == skr::input::INPUT_RESULT_OK && input_reading &&
            input_layer->GetMouseState(input_reading, &mouse_state))
        {
            // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
            {
                ZoneScopedN("UpdateMouseButton");
                io.MouseDown[0] = mouse_state.buttons & EMouseKey::MOUSE_KEY_LB;
                io.MouseDown[1] = mouse_state.buttons & EMouseKey::MOUSE_KEY_RB;
                io.MouseDown[2] = mouse_state.buttons & EMouseKey::MOUSE_KEY_MB;
            }
            input_layer->Release(input_reading);
        }
    }
    else // fallback
    {
        ZoneScopedN("UpdateMouseButton-Fallback");
        // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        io.MouseDown[0] = skr_mouse_key_down(EMouseKey::MOUSE_KEY_LB);
        io.MouseDown[1] = skr_mouse_key_down(EMouseKey::MOUSE_KEY_RB);
        io.MouseDown[2] = skr_mouse_key_down(EMouseKey::MOUSE_KEY_MB);
    }

    // Update viewport events
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ZoneScopedN("UpdateMouseEvents-1");
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
        ZoneScopedN("UpdateMouseEvents-2");
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
        ZoneScopedN("UpdateViewportEvents");

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

static void ImGui_UpdateKeyModifiers()
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiKey_ModCtrl, false);
    io.AddKeyEvent(ImGuiKey_ModShift, false);
    io.AddKeyEvent(ImGuiKey_ModAlt, false);
    io.AddKeyEvent(ImGuiKey_ModSuper, false);
}

void skr_imgui_initialize(skr_system_handler_id handler)
{
    auto rid = handler->add_window_close_handler(
        +[](SWindowHandle window, void*) {
            if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)window))
            {
                viewport->PlatformRequestClose = true;
            }
        }, nullptr);
    rid = handler->add_window_move_handler(
        +[](SWindowHandle window, void*) {
            if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)window))
            {
                viewport->PlatformRequestMove = true;
            }
        }, nullptr);
    rid = handler->add_window_resize_handler(
        +[](SWindowHandle window, int32_t w, int32_t h, void*) {
            if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle((void*)window))
            {
                viewport->PlatformRequestResize = true;
            }
        }, nullptr);
    rid = handler->add_mouse_wheel_handler(
        +[](int32_t X, int32_t Y, void* usr_data) {
            ImGuiIO& io = ImGui::GetIO();
            io.AddMouseWheelEvent((float)X, (float)Y);
        }, nullptr);
    rid = handler->add_mouse_button_down_handler(
        +[](EMouseKey key, int32_t X, int32_t Y, void* usr_data) {
            int mouse_button = -1;
            if (key == EMouseKey::MOUSE_KEY_LB) { mouse_button = 0; }
            if (key == EMouseKey::MOUSE_KEY_RB) { mouse_button = 1; }
            if (key == EMouseKey::MOUSE_KEY_MB) { mouse_button = 2; }
            if (key == EMouseKey::MOUSE_KEY_X1B) { mouse_button = 3; }
            if (key == EMouseKey::MOUSE_KEY_X2B) { mouse_button = 4; }

            ImGuiIO& io = ImGui::GetIO();
            io.AddMouseButtonEvent(mouse_button, true);
        }, nullptr);
    rid = handler->add_mouse_button_up_handler(
        +[](EMouseKey key, int32_t X, int32_t Y, void* usr_data) {
            int mouse_button = -1;
            if (key == EMouseKey::MOUSE_KEY_LB) { mouse_button = 0; }
            if (key == EMouseKey::MOUSE_KEY_RB) { mouse_button = 1; }
            if (key == EMouseKey::MOUSE_KEY_MB) { mouse_button = 2; }
            if (key == EMouseKey::MOUSE_KEY_X1B) { mouse_button = 3; }
            if (key == EMouseKey::MOUSE_KEY_X2B) { mouse_button = 4; }

            ImGuiIO& io = ImGui::GetIO();
            io.AddMouseButtonEvent(mouse_button, false);
        }, nullptr);
    rid = handler->add_key_down_handler(
        +[](EKeyCode key, void* usr_data) {
            ImGuiIO& io = ImGui::GetIO();
            ImGui_UpdateKeyModifiers();
            const auto k = skr::imgui::KeyCodeTranslator(key);
            io.AddKeyEvent(k, true);
        }, nullptr);
    rid = handler->add_key_up_handler(
        +[](EKeyCode key, void* usr_data) {
            ImGuiIO& io = ImGui::GetIO();
            ImGui_UpdateKeyModifiers();
            const auto k = skr::imgui::KeyCodeTranslator(key);
            io.AddKeyEvent(k, false);
        }, nullptr);
    rid = handler->add_text_input_handler(
        +[](const char8_t* text, void* usr_data) {
            ImGuiIO& io = ImGui::GetIO();
            io.AddInputCharactersUTF8((const char*)text);
        }, nullptr);
    (void)rid;
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
        ZoneScopedN("UpdateMonitors");

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
        eastl::fixed_vector<SMonitorHandle, 4> skr_monitors; // 4 inline monitors that I think is enough for non-allocation.
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
        ZoneScopedN("UpdateInput");

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
    skr::string title = "imgui-";
    title += skr::to_string(viewport->ID);
    auto new_window = skr_create_window((const char8_t*)title.c_str(), &desc);

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
    skr_window_set_title((SWindowHandle)viewport->PlatformHandle, (const char8_t*)title);
}

void skr::imgui::imgui_set_window_alpha(ImGuiViewport* viewport, float alpha)
{
    SKR_LOG_WARN("imgui_set_window_alpha not implemented");

}

void skr::imgui::imgui_set_window_focus(ImGuiViewport* viewport)
{
    SKR_LOG_TRACE("imgui_set_window_focus not implemented");
    
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

#define KEY_CODE_TRANS(k, imguik) case (k): return (imguik);

inline static int32_t skr::imgui::KeyCodeTranslator(EKeyCode keycode)
{
    switch (keycode)
    {
        KEY_CODE_TRANS(KEY_CODE_Invalid, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_Backspace, ImGuiKey_Backspace)
        KEY_CODE_TRANS(KEY_CODE_Tab, ImGuiKey_Tab)
        KEY_CODE_TRANS(KEY_CODE_Clear, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_Enter, ImGuiKey_Enter)
        // combined
        KEY_CODE_TRANS(KEY_CODE_Pause, ImGuiKey_Pause)
        KEY_CODE_TRANS(KEY_CODE_CapsLock, ImGuiKey_CapsLock)
        KEY_CODE_TRANS(KEY_CODE_Esc, ImGuiKey_Escape)
        KEY_CODE_TRANS(KEY_CODE_SpaceBar, ImGuiKey_Space)
        KEY_CODE_TRANS(KEY_CODE_PageUp, ImGuiKey_PageUp)
        KEY_CODE_TRANS(KEY_CODE_PageDown, ImGuiKey_PageDown)
        KEY_CODE_TRANS(KEY_CODE_End, ImGuiKey_End)
        KEY_CODE_TRANS(KEY_CODE_Home, ImGuiKey_Home)
        KEY_CODE_TRANS(KEY_CODE_Left, ImGuiKey_LeftArrow)
        KEY_CODE_TRANS(KEY_CODE_Up, ImGuiKey_UpArrow)
        KEY_CODE_TRANS(KEY_CODE_Right, ImGuiKey_RightArrow)
        KEY_CODE_TRANS(KEY_CODE_Down, ImGuiKey_DownArrow)
        KEY_CODE_TRANS(KEY_CODE_Select, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_Execute, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_Printscreen, ImGuiKey_PrintScreen)
        KEY_CODE_TRANS(KEY_CODE_Insert, ImGuiKey_Insert)
        KEY_CODE_TRANS(KEY_CODE_Del, ImGuiKey_Delete)
        KEY_CODE_TRANS(KEY_CODE_Help, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_0, ImGuiKey_0)
        KEY_CODE_TRANS(KEY_CODE_1, ImGuiKey_1)
        KEY_CODE_TRANS(KEY_CODE_2, ImGuiKey_2)
        KEY_CODE_TRANS(KEY_CODE_3, ImGuiKey_3)
        KEY_CODE_TRANS(KEY_CODE_4, ImGuiKey_4)
        KEY_CODE_TRANS(KEY_CODE_5, ImGuiKey_5)
        KEY_CODE_TRANS(KEY_CODE_6, ImGuiKey_6)
        KEY_CODE_TRANS(KEY_CODE_7, ImGuiKey_7)
        KEY_CODE_TRANS(KEY_CODE_8, ImGuiKey_8)
        KEY_CODE_TRANS(KEY_CODE_9, ImGuiKey_9)
        KEY_CODE_TRANS(KEY_CODE_A, ImGuiKey_A)
        KEY_CODE_TRANS(KEY_CODE_B, ImGuiKey_B)
        KEY_CODE_TRANS(KEY_CODE_C, ImGuiKey_C)
        KEY_CODE_TRANS(KEY_CODE_D, ImGuiKey_D)
        KEY_CODE_TRANS(KEY_CODE_E, ImGuiKey_E)
        KEY_CODE_TRANS(KEY_CODE_F, ImGuiKey_F)
        KEY_CODE_TRANS(KEY_CODE_G, ImGuiKey_G)
        KEY_CODE_TRANS(KEY_CODE_H, ImGuiKey_H)
        KEY_CODE_TRANS(KEY_CODE_I, ImGuiKey_I)
        KEY_CODE_TRANS(KEY_CODE_J, ImGuiKey_J)
        KEY_CODE_TRANS(KEY_CODE_K, ImGuiKey_K)
        KEY_CODE_TRANS(KEY_CODE_L, ImGuiKey_L)
        KEY_CODE_TRANS(KEY_CODE_M, ImGuiKey_M)
        KEY_CODE_TRANS(KEY_CODE_N, ImGuiKey_N)
        KEY_CODE_TRANS(KEY_CODE_O, ImGuiKey_O)
        KEY_CODE_TRANS(KEY_CODE_P, ImGuiKey_P)
        KEY_CODE_TRANS(KEY_CODE_Q, ImGuiKey_Q)
        KEY_CODE_TRANS(KEY_CODE_R, ImGuiKey_R)
        KEY_CODE_TRANS(KEY_CODE_S, ImGuiKey_S)
        KEY_CODE_TRANS(KEY_CODE_T, ImGuiKey_T)
        KEY_CODE_TRANS(KEY_CODE_U, ImGuiKey_U)
        KEY_CODE_TRANS(KEY_CODE_V, ImGuiKey_V)
        KEY_CODE_TRANS(KEY_CODE_W, ImGuiKey_W)
        KEY_CODE_TRANS(KEY_CODE_X, ImGuiKey_X)
        KEY_CODE_TRANS(KEY_CODE_Y, ImGuiKey_Y)
        KEY_CODE_TRANS(KEY_CODE_Z, ImGuiKey_Z)
        KEY_CODE_TRANS(KEY_CODE_App, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_Sleep, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_Numpad0, ImGuiKey_0)
        KEY_CODE_TRANS(KEY_CODE_Numpad1, ImGuiKey_1)
        KEY_CODE_TRANS(KEY_CODE_Numpad2, ImGuiKey_2)
        KEY_CODE_TRANS(KEY_CODE_Numpad3, ImGuiKey_3)
        KEY_CODE_TRANS(KEY_CODE_Numpad4, ImGuiKey_4)
        KEY_CODE_TRANS(KEY_CODE_Numpad5, ImGuiKey_5)
        KEY_CODE_TRANS(KEY_CODE_Numpad6, ImGuiKey_6)
        KEY_CODE_TRANS(KEY_CODE_Numpad7, ImGuiKey_7)
        KEY_CODE_TRANS(KEY_CODE_Numpad8, ImGuiKey_8)
        KEY_CODE_TRANS(KEY_CODE_Numpad9, ImGuiKey_9)
        KEY_CODE_TRANS(KEY_CODE_Multiply, ImGuiKey_KeypadMultiply)
        KEY_CODE_TRANS(KEY_CODE_Add, ImGuiKey_KeypadAdd)
        KEY_CODE_TRANS(KEY_CODE_Separator, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_Subtract, ImGuiKey_KeypadSubtract)
        KEY_CODE_TRANS(KEY_CODE_Decimal, ImGuiKey_KeypadDecimal)
        KEY_CODE_TRANS(KEY_CODE_Divide, ImGuiKey_KeypadDivide)
        KEY_CODE_TRANS(KEY_CODE_F1, ImGuiKey_F1)
        KEY_CODE_TRANS(KEY_CODE_F2, ImGuiKey_F2)
        KEY_CODE_TRANS(KEY_CODE_F3, ImGuiKey_F3)
        KEY_CODE_TRANS(KEY_CODE_F4, ImGuiKey_F4)
        KEY_CODE_TRANS(KEY_CODE_F5, ImGuiKey_F5)
        KEY_CODE_TRANS(KEY_CODE_F6, ImGuiKey_F6)
        KEY_CODE_TRANS(KEY_CODE_F7, ImGuiKey_F7)
        KEY_CODE_TRANS(KEY_CODE_F8, ImGuiKey_F8)
        KEY_CODE_TRANS(KEY_CODE_F9, ImGuiKey_F9)
        KEY_CODE_TRANS(KEY_CODE_F10, ImGuiKey_F10)
        KEY_CODE_TRANS(KEY_CODE_F11, ImGuiKey_F11)
        KEY_CODE_TRANS(KEY_CODE_F12, ImGuiKey_F12)
        KEY_CODE_TRANS(KEY_CODE_F13, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_F14, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_F15, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_F16, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_F17, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_F18, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_F19, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_F20, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_F21, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_F22, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_F23, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_F24, ImGuiKey_None)
        KEY_CODE_TRANS(KEY_CODE_Numlock, ImGuiKey_NumLock)
        KEY_CODE_TRANS(KEY_CODE_Scrolllock, ImGuiKey_ScrollLock)

        KEY_CODE_TRANS(KEY_CODE_LShift, ImGuiKey_LeftShift)
        KEY_CODE_TRANS(KEY_CODE_RShift, ImGuiKey_RightShift)
        KEY_CODE_TRANS(KEY_CODE_LCtrl, ImGuiKey_LeftCtrl)
        KEY_CODE_TRANS(KEY_CODE_RCtrl, ImGuiKey_RightCtrl)
        KEY_CODE_TRANS(KEY_CODE_LAlt, ImGuiKey_LeftAlt)
        KEY_CODE_TRANS(KEY_CODE_RAlt, ImGuiKey_RightAlt)

        KEY_CODE_TRANS(KEY_CODE_Semicolon, ImGuiKey_Semicolon)
        KEY_CODE_TRANS(KEY_CODE_Plus, ImGuiKey_Equal)
        KEY_CODE_TRANS(KEY_CODE_Comma, ImGuiKey_Comma)
        KEY_CODE_TRANS(KEY_CODE_Minus, ImGuiKey_Minus)
        KEY_CODE_TRANS(KEY_CODE_Dot, ImGuiKey_Period)
        KEY_CODE_TRANS(KEY_CODE_Slash, ImGuiKey_Slash)
        KEY_CODE_TRANS(KEY_CODE_Wave, ImGuiKey_GraveAccent)
        KEY_CODE_TRANS(KEY_CODE_LBracket, ImGuiKey_LeftBracket)
        KEY_CODE_TRANS(KEY_CODE_Backslash, ImGuiKey_Backslash)
        KEY_CODE_TRANS(KEY_CODE_RBracket, ImGuiKey_RightBracket)
        KEY_CODE_TRANS(KEY_CODE_Quote, ImGuiKey_Apostrophe)

        default:
            return KEY_CODE_Invalid;
    }
}

#undef KEY_CODE_TRANS