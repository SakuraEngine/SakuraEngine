#pragma once
#include "platform/window.h"
#include "SkrImGui/skr_imgui.config.h"
#include "imgui/imgui.h"

SKR_DECLARE_TYPE_ID_FWD(skr, ISystemHandler, skr_system_handler);
struct lua_State;

RUNTIME_EXTERN_C SKR_IMGUI_API 
void skr_imgui_initialize(skr_system_handler_id handler);

RUNTIME_EXTERN_C SKR_IMGUI_API 
void skr_imgui_new_frame(SWindowHandle window, float delta_time);

SKR_IMGUI_EXTERN_C SKR_IMGUI_API 
void skr_lua_bind_imgui(lua_State* L);