#pragma once
#include <imgui.h>
#include <SkrContainers/string.hpp>

namespace ImGui
{
SKR_IMGUI_API bool InputText(const char* label, skr::String* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
SKR_IMGUI_API bool InputTextMultiline(const char* label, skr::String* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
SKR_IMGUI_API bool InputTextWithHint(const char* label, const char* hint, skr::String* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
} // namespace ImGui
