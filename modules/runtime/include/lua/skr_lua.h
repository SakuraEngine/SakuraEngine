#pragma once
#include "platform/configure.h"
#include "lua/lua.hpp"
#include "utils/types.h"

RUNTIME_EXTERN_C RUNTIME_API lua_State *skr_lua_newstate(struct skr_vfs_t* vfs);
RUNTIME_EXTERN_C RUNTIME_API void skr_lua_close(lua_State* L);


