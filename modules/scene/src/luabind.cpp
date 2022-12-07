#include "SkrScene/scene.h"

namespace skr::lua
{
    int push_name_comp(lua_State *L, const skr_name_comp_t &value)
    {
        lua_pushstring(L, value.str);
        return 1;
    }

    skr_name_comp_t check_name_comp(lua_State *L, int index)
    {
        skr_name_comp_t value;
        size_t len;
        auto string = luaL_checklstring(L, index, &len);
        SKR_ASSERT(len <= SKR_SCENE_MAX_NAME_LENGTH);
        len = std::min<size_t>(len, SKR_SCENE_MAX_NAME_LENGTH);
        memcpy(value.str, string, len);
        return value;
    }
}