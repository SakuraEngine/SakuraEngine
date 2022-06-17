#include "imgui/imgui.h"
#include "imgui/skr_imgui.h"

namespace skr::imgui
{
RUNTIME_API ImGuiContext*& imgui_context()
{
    static thread_local ImGuiContext* ctx = nullptr;
    return ctx;
}
} // namespace skr::imgui

static void imgui_update_mouse_and_buttons(SWindowHandle window)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseHoveredViewport = 0;

    // [1]
    // Only when requested by io.WantSetMousePos: set OS mouse pos from Dear ImGui mouse pos.
    // (rarely used, mostly when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos)
    {
        skr_set_cursor_pos((uint32_t)io.MousePos.x, (uint32_t)io.MousePos.y);
    }
    else
    {
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    }

    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    io.MouseHoveredViewport = 0;

    int32_t pos_x, pos_y;
    skr_cursor_pos(&pos_x, &pos_y);
    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    io.MouseDown[0] = skr_mouse_key_down(EMouseKey::MOUSE_KEY_LB);
    io.MouseDown[1] = skr_mouse_key_down(EMouseKey::MOUSE_KEY_RB);
    io.MouseDown[2] = skr_mouse_key_down(EMouseKey::MOUSE_KEY_MB);

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        // Multi-viewport mode: mouse position in OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the primary monitor)
        // This is the position you can get with GetCursorPos(). In theory adding viewport->Pos is also the reverse operation of doing ScreenToClient().
        if (ImGui::FindViewportByPlatformHandle(skr_window_get_native_handle(window)) != NULL)
            io.MousePos = ImVec2((float)pos_x, (float)pos_y);
    }
    else
    {
        // Single viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window.)
        // This is the position you can get with GetCursorPos() + ScreenToClient() or from WM_MOUSEMOVE.
        if (skr_get_mouse_focused_window() == window)
        {
            io.MousePos = ImVec2((float)pos_x, (float)pos_y);
        }
    }
}

void skr_imgui_new_frame(SWindowHandle window, float delta_time)
{
    ImGuiIO& io = ImGui::GetIO();

    int extent_w, extent_h;
    skr_window_get_extent(window, &extent_w, &extent_h);
    io.DisplaySize = { static_cast<float>(extent_w), static_cast<float>(extent_h) };

    // !! no Scalable Drawing Support.
    // TODO: Support.
    io.DisplayFramebufferScale = ImVec2(1, 1);

    // set delta time.
    io.DeltaTime = delta_time;

    imgui_update_mouse_and_buttons(window);

    ImGui::NewFrame();
}