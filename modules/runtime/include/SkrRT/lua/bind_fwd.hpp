#pragma once
#include "misc/types.h"
struct lua_State;
namespace skr::lua
{
    template<class T, class = void>
    struct BindTrait;
}