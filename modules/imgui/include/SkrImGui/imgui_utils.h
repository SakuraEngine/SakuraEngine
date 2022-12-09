#pragma once

#include "containers/string.hpp"
#include "skr_imgui.h"

namespace ImGui
{
    // ImGui::InputText() with skr::string
    // Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
    SKR_IMGUI_API bool  InputText(const char* label, skr::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    SKR_IMGUI_API bool  InputTextMultiline(const char* label, skr::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    SKR_IMGUI_API bool  InputTextWithHint(const char* label, const char* hint, skr::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
}
