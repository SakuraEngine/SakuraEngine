#pragma once
#include "dual.h"
#include "lua/bind.hpp"

namespace dual
{
    template<class T>
    void SetLuaBindCallback(dual_type_description_t& desc)
    {
        if constexpr(skr::is_complete_v<skr::lua::BindTrait<T>>)
        {
            desc.callback.lua_push = +[](dual_chunk_t* chunk, EIndex index, char* data, struct lua_State* L) -> int
            {
                return skr::lua::BindTrait<T>::push(L, *(T*)data);
            };
            desc.callback.lua_check = +[](dual_chunk_t* chunk, EIndex index, char* data, struct lua_State* L, int lua_index)
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