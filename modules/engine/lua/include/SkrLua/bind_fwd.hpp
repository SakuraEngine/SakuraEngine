#pragma once
#include "SkrBase/config.h"

struct lua_State;
namespace skr::lua
{
template <class T, class = void>
struct BindTrait;
}