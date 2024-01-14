#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrLua/bind.hpp"

namespace sugoi
{
    template<class T>
    void SetLuaBindCallback(sugoi_type_description_t& desc)
    {
        if constexpr(skr::is_complete_v<skr::lua::BindTrait<T>>)
        {
            desc.callback.lua_push = +[](sugoi_chunk_t* chunk, EIndex index, char* data, struct lua_State* L) -> int
            {
                return skr::lua::BindTrait<T>::push(L, *(T*)data);
            };
            desc.callback.lua_check = +[](sugoi_chunk_t* chunk, EIndex index, char* data, struct lua_State* L, int lua_index)
            {
                *(T*)data = skr::lua::BindTrait<T>::check(L, lua_index);
            };
        }
        else
        {
            desc.callback.lua_push = nullptr;
            desc.callback.lua_check = nullptr;
        }
    }
}