#pragma once
#include "SkrRT/misc/types.h"
#include "SkrLua/module.configure.h"

struct lua_State;
namespace skr::lua
{
    template<class T, class = void>
    struct BindTrait;
}