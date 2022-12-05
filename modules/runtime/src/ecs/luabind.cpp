#include "lua/lua.hpp"
#include "stack.hpp"
#include "query.hpp"
#include "chunk.hpp"
#include "archetype.hpp"
#include "type_registry.hpp"
namespace dual
{
extern thread_local fixed_stack_t localStack;
}
namespace skr::lua
{
    struct lua_chunk_view_t {
        dual_chunk_view_t view;
        dual_query_t* query;
        uint32_t* localIndices;
        const char** guidStrs;
    };

    void bind_ecs(lua_State* L)
    {
        lua_getglobal(L, "skr");
        // bind create query
        {
            auto trampoline = +[](lua_State* L) -> int {
                dual_storage_t* storage = (dual_storage_t*)luaL_checkudata(L, 1, "skr_opaque_t");
                const char* literal = luaL_checkstring(L, 2);
                auto query = dualQ_from_literal(storage, literal);
                lua_pushlightuserdata(L, query);
                //luaL_getmetatable(L, "skr_opaque_t");
                //lua_setmetatable(L, -2);
                return 1;
            };
            lua_pushcfunction(L, trampoline);
            lua_setfield(L, -2, "create_query");
        }
        
        // bind iterate query
        {
            auto trampoline = +[](lua_State* L) -> int {
                auto query = (dual_query_t*)lua_touserdata(L, 1);
                if(!query) return 0;
                luaL_argexpected(L, lua_isfunction(L, 2), 2, "function");
                dual_view_callback_t callback = +[](void* userdata, dual_chunk_view_t* view) -> void {
                    lua_State* L = (lua_State*)userdata;
                    dual_query_t* query = (dual_query_t*)lua_touserdata(L, 1);
                    dual::fixed_stack_scope_t scope(dual::localStack);
                    lua_chunk_view_t luaView { *view, query };
                    luaView.localIndices = dual::localStack.allocate<uint32_t>(query->parameters.length);
                    luaView.guidStrs = dual::localStack.allocate<const char*>(query->parameters.length);
                    auto& typeReg = dual::type_registry_t::get();
                    forloop(i, 0, query->parameters.length)
                    {
                        luaView.localIndices[i] = view->chunk->group->index(query->parameters.types[i]);
                        luaView.guidStrs[i] = typeReg.descriptions[dual::type_index_t(query->parameters.types[i]).index()].guidStr;
                    }
                    lua_pushvalue(L, 2);
                    *(lua_chunk_view_t**)lua_newuserdata(L, sizeof(void*)) = &luaView;
                    luaL_getmetatable(L, "lua_chunk_view_t");
                    lua_setmetatable(L, -2);
                    if(lua_pcall(L, 1, 0, 0) != LUA_OK)
                    {
                        lua_getglobal(L, "skr");
                        lua_getfield(L, -1, "log_error");
                        lua_pushvalue(L, -3);
                        lua_call(L, 1, 0);
                        lua_pop(L, 2);
                    }
                };
                void* u = (void*)L;
                dualQ_get_views(query, callback, u);
                return 0;
            };
            lua_pushcfunction(L, trampoline);
            lua_setfield(L, -2, "iterate_query");
        }
        lua_pop(L, 1);

        // bind chunk view
        luaL_Reg chunkViewMethods[] = {
            { "__index", +[](lua_State* L) -> int {
                lua_chunk_view_t* view = *(lua_chunk_view_t**)luaL_checkudata(L, 1, "lua_chunk_view_t");
                if(lua_isnumber(L, 2))
                {
                    uint32_t index = lua_tointeger(L, 2);
                    luaL_argexpected(L, index < view->view.count, 2, "index out of bounds");
                    forloop(i, 0, view->query->parameters.length)
                    {
                        uint32_t localIndex = view->localIndices[i];
                        if(localIndex == UINT32_MAX)
                        {
                            lua_pushnil(L);
                        }
                        else
                        {
                            void* data;
                            if(view->query->parameters.accesses[i].readonly)
                            {
                                data = (void*)dualV_get_owned_ro_local(&view->view, view->localIndices[i]);
                            }
                            else {
                                data = dualV_get_owned_rw_local(&view->view, view->localIndices[i]);
                            }
                            *(void**)lua_newuserdata(L, sizeof(void*)) = data;
                            luaL_getmetatable(L, view->guidStrs[i]);
                            if(lua_isnil(L, -1))
                                luaL_getmetatable(L, "skr_opaque_t");
                            lua_setmetatable(L, -2);
                        }
                    }
                    return view->query->parameters.length;
                }
                else if(lua_isstring(L, 2))
                {
                    const char* name = lua_tostring(L, 2);
                    if(strcmp(name, "length") == 0)
                    {
                        lua_pushinteger(L, view->view.count);
                        return 1;
                    }
                    else 
                    {
                        return luaL_error(L, "invalid chunk view field '%s'", name);
                    }
                }
                else
                {
                    return luaL_error(L, "invalid chunk view index");
                }
            } },
            { NULL, NULL }
        };
        luaL_newmetatable(L, "lua_chunk_view_t");
        luaL_setfuncs(L, chunkViewMethods, 0);
        lua_pop(L, 1);
    }
}