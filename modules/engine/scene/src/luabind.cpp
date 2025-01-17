#include "SkrScene/scene.h"

namespace skr::lua
{
// FIXME. lua support
// int push_name_comp(lua_State *L, const NameComponent &value)
// {
//     lua_pushstring(L, value.str);
//     return 1;
// }

// NameComponent check_name_comp(lua_State *L, int index)
// {
//     NameComponent value;
//     size_t len;
//     auto string = luaL_checklstring(L, index, &len);
//     SKR_ASSERT(len <= SKR_SCENE_MAX_NAME_LENGTH);
//     len = std::min<size_t>(len, SKR_SCENE_MAX_NAME_LENGTH);
//     memcpy(value.str, string, len);
//     return value;
// }
} // namespace skr::lua