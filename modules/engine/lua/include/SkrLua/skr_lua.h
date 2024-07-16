#pragma once
#include "SkrRT/config.h"
#include "SkrBase/config.h"
extern "C" {
#include "lua.h"
#include "lualib.h"
}

SKR_EXTERN_C SKR_LUA_API lua_State* skr_lua_newstate(struct skr_vfs_t* vfs);
SKR_EXTERN_C SKR_LUA_API int        skr_lua_loadfile(lua_State* L, const char* filename);
SKR_EXTERN_C SKR_LUA_API void       skr_lua_close(lua_State* L);
