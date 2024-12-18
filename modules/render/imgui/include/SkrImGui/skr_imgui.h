#pragma once
#include "SkrCore/platform/window.h"
#include "SkrImGui/skr_imgui.config.h"
#include "imgui/imgui.h" // IWYU pragma: export

SKR_DECLARE_TYPE_ID_FWD(skr, ISystemHandler, skr_system_handler);
struct lua_State;

SKR_EXTERN_C SKR_IMGUI_API 
void skr_imgui_initialize(skr_system_handler_id handler);

SKR_EXTERN_C SKR_IMGUI_API 
void skr_imgui_new_frame(SWindowHandle window, float delta_time);

SKR_IMGUI_EXTERN_C SKR_IMGUI_API 
void skr_lua_bind_imgui(lua_State* L);