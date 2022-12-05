#pragma once
#include "platform/window.h"
#include "SkrImGui/skr_imgui.config.h"
#include "imgui/imgui.h"

SKR_IMGUI_API void skr_imgui_new_frame(SWindowHandle window, float delta_time);

struct lua_State;
SKR_IMGUI_EXTERN_C SKR_IMGUI_API void skr_lua_bind_imgui(lua_State* L);