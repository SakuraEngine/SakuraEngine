#pragma once
#include "platform/configure.h"
extern "C" {
#include "lua.h"
#include "lualib.h"
}
#include "misc/types.h"

RUNTIME_EXTERN_C RUNTIME_API lua_State *skr_lua_newstate(struct skr_vfs_t* vfs);
RUNTIME_EXTERN_C RUNTIME_API void skr_lua_close(lua_State* L);


