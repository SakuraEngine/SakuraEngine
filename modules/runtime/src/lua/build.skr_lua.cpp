#include "lua/skr_lua.h"
#include "platform/memory.h"
#include "utils/types.h"
#include "utils/format.hpp"
#include "platform/guid.hpp"
#include "resource/resource_handle.h"
#include "lua/bind.hpp"
#include "utils/log.h"

lua_State *skr_lua_newstate()
{
    lua_State* L = lua_newstate(+[](void* ud, void* ptr, size_t osize, size_t nsize) -> void*
    {
        if (nsize == 0)
        {
            sakura_free(ptr);
            return nullptr;
        }
        else if(osize == 0)
        {
            return sakura_malloc(nsize);
        }
        else
        {
            return sakura_realloc(ptr, nsize);
        }
    }, nullptr);

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "skr");
    //resource constructor
    lua_pushcfunction(L, +[](lua_State* L) -> int
    {
        if(luaL_testudata(L, 1, "skr_guid_t"))
        {
            const skr_guid_t* guid = skr::lua::check_guid(L, 1);
            skr_resource_handle_t* resource = (skr_resource_handle_t*)lua_newuserdata(L, sizeof(skr_resource_handle_t));
            new(resource) skr_resource_handle_t(*guid);
            luaL_setmetatable(L, "skr_resource_handle_t");
            return 1;
        }
        else if(lua_isstring(L, 1))
        {
            const char* str = lua_tostring(L, 1);
            skr_resource_handle_t* resource = (skr_resource_handle_t*)lua_newuserdata(L, sizeof(skr_resource_handle_t));
            new(resource) skr_resource_handle_t(skr::guid::make_guid_unsafe(skr::string_view{str}));
            luaL_setmetatable(L, "skr_resource_handle_t");
            return 1;
        }
        else
        {
            return luaL_error(L, "invalid arguments for skr_resource_handle_t constructor");
        }
    });
    lua_setfield(L, -2, "resource_handle");
    lua_pop(L, 1);

    {
        luaL_newmetatable(L, "skr_guid_t");
        int metatable = lua_gettop(L);
        lua_pushstring(L, "__tostring");
        lua_pushcfunction(L, +[](lua_State* L) -> int
        {
            auto guid = (skr_guid_t*)luaL_checkudata(L, 1, "skr_guid_t");
            lua_pushstring(L, skr::format("{}", *guid).c_str());
            return 1;
        });
        lua_settable(L, metatable);
        lua_pushstring(L, "__eq");
        lua_pushcfunction(L, +[](lua_State* L) -> int
        {
            auto guid1 = (skr_guid_t*)luaL_checkudata(L, 1, "skr_guid_t");
            auto guid2 = (skr_guid_t*)luaL_checkudata(L, 2, "skr_guid_t");
            lua_pushboolean(L, *guid1 == *guid2);
            return 1;
        });
        lua_settable(L, metatable);
    }

    {
        luaL_newmetatable(L, "skr_resource_handle_t");
        int metatable = lua_gettop(L);
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, +[](lua_State* L) -> int
        {
            auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
            if(resource->is_resolved())
                resource->unload();
            return 0;
        });
        lua_pushstring(L, "__tostring");
        lua_pushcfunction(L, +[](lua_State* L) -> int
        {
            auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
            lua_pushstring(L, skr::format("resource {}", resource->get_serialized()).c_str());
            return 1;
        });
        lua_settable(L, metatable);
        lua_pushstring(L, "__eq");
        lua_pushcfunction(L, +[](lua_State* L) -> int
        {
            auto resource1 = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
            auto resource2 = (skr_resource_handle_t*)luaL_checkudata(L, 2, "skr_resource_handle_t");
            lua_pushboolean(L, resource1->get_serialized() == resource2->get_serialized());
            return 1;
        });
        lua_settable(L, metatable);
        lua_pushstring(L, "resolve");
        lua_pushcfunction(L, +[](lua_State* L) -> int
        {
            auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
            bool requireInstall = lua_toboolean(L, 2);
            if(!resource->is_resolved())
                resource->resolve(requireInstall, (uint64_t)L, SKR_REQUESTER_SCRIPT);
            return 0;
        });
        lua_settable(L, metatable);
        lua_pushstring(L, "is_resolved");
        lua_pushcfunction(L, +[](lua_State* L) -> int
        {
            auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
            lua_pushboolean(L, resource->is_resolved());
            return 1;
        });
        lua_settable(L, metatable);
        lua_pushstring(L, "get_resolved");
        lua_pushcfunction(L, +[](lua_State* L) -> int
        {
            auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
            if(!resource->is_resolved())
            {
                return 0; 
            }
            auto ptr = resource->get_resolved();
            if(!ptr)
            {
                return 0;
            }
            auto tid = resource->get_type();
            auto type = skr_get_type(&tid);
            lua_pushlightuserdata(L, ptr);
            luaL_getmetatable(L, type->Name());
            lua_setmetatable(L, -2);
            return 1;
        });
        lua_settable(L, metatable);
        lua_pushstring(L, "unload");
        lua_pushcfunction(L, +[](lua_State* L) -> int
        {
            auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
            if(resource->is_resolved())
                resource->unload();
            else
                SKR_LOG_DEBUG("skr_resource_handle_t::unload called on unresolved resource.");
            return 0;
        });
        lua_settable(L, metatable);
    }

    return L;
}

namespace skr::lua
{
    int push_guid(lua_State* L, const skr_guid_t* guid)
    {
        auto ud = (skr_guid_t*)lua_newuserdata(L, sizeof(skr_guid_t));
        *ud = *guid;
        luaL_getmetatable(L, "skr_guid_t");
        lua_setmetatable(L, -2);
        return 1;
    }

    const skr_guid_t* check_guid(lua_State* L, int index)
    {
        return (skr_guid_t*)luaL_checkudata(L, index, "skr_guid_t");
    }

    int push_string(lua_State* L, const skr::string& str)
    {
        lua_pushstring(L, str.c_str());
        return 1;
    }

    int push_string(lua_State* L, skr::string_view str)
    {
        lua_pushlstring(L, str.data(), str.size());
        return 1;
    }

    skr::string check_string(lua_State* L, int index)
    {
        return lua_tostring(L, index);
    }

    int push_resource(lua_State* L, const skr_resource_handle_t* resource)
    {
        auto ud = (skr_resource_handle_t*)lua_newuserdata(L, sizeof(skr_resource_handle_t));
        new (ud) skr_resource_handle_t(*resource, (uint64_t)L, SKR_REQUESTER_SCRIPT);
        luaL_getmetatable(L, "skr_resource_handle_t");
        lua_setmetatable(L, -2);
        return 1;
    }

    const skr_resource_handle_t* check_resource(lua_State* L, int index)
    {
        return (skr_resource_handle_t*)luaL_checkudata(L, index, "skr_resource_handle_t");
    }
}