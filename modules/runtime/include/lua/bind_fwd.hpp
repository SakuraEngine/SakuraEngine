#pragma once
#include "utils/types.h"
struct lua_State;
namespace skr::lua
{
    template<class T, class = void>
    struct BindTrait;
}