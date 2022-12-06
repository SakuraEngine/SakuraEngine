#include "lua/lua.hpp"
#include "stack.hpp"
#include "query.hpp"
#include "chunk.hpp"
#include "archetype.hpp"
#include "type_registry.hpp"
#include "ecs/type_builder.hpp"
#include "ecs/callback.hpp"
namespace dual
{
extern thread_local fixed_stack_t localStack;
}
namespace skr::lua
{
    struct lua_chunk_view_t {
        dual_chunk_view_t view;
        uint32_t count;
        const dual_entity_t* entities;
        void** datas;
        uint32_t* strides;
        const char** guidStrs;
    };

    static lua_chunk_view_t init_chunk_view(dual_chunk_view_t* view, dual_entity_type_t& type)
    {
        lua_chunk_view_t luaView { *view, type.type.length };

        luaView.datas = dual::localStack.allocate<void*>(type.type.length);
        luaView.strides = dual::localStack.allocate<uint32_t>(type.type.length);
        luaView.guidStrs = dual::localStack.allocate<const char*>(type.type.length);
        luaView.entities = dualV_get_entities(view);
        auto& typeReg = dual::type_registry_t::get();
        forloop(i, 0, type.type.length)
        {
            luaView.datas[i] = (void*)dualV_get_owned_ro(view, type.type.data[i]);
            auto& desc = typeReg.descriptions[dual::type_index_t(type.type.data[i]).index()];
            luaView.strides[i] = desc.size;
            luaView.guidStrs[i] = desc.guidStr;
        }
        return luaView;
    }

    static lua_chunk_view_t query_chunk_view(dual_chunk_view_t* view, dual_query_t* query)
    {
        lua_chunk_view_t luaView { *view, query->parameters.length };
        luaView.datas = dual::localStack.allocate<void*>(query->parameters.length);
        luaView.strides = dual::localStack.allocate<uint32_t>(query->parameters.length);
        luaView.guidStrs = dual::localStack.allocate<const char*>(query->parameters.length);
        luaView.entities = dualV_get_entities(view);
        auto& typeReg = dual::type_registry_t::get();
        forloop(i, 0, query->parameters.length)
        {
            if(query->parameters.accesses[i].readonly)
                luaView.datas[i] = (void*)dualV_get_owned_ro(view, query->parameters.types[i]);
            else
                luaView.datas[i] = (void*)dualV_get_owned_rw(view, query->parameters.types[i]);
            auto& desc = typeReg.descriptions[dual::type_index_t(query->parameters.types[i]).index()];
            luaView.strides[i] = desc.size;
            luaView.guidStrs[i] = desc.guidStr;
        }
        return luaView;
    }

    void bind_ecs(lua_State* L)
    {
        lua_getglobal(L, "skr");

        //bind component
        {
            auto trampoline = +[](lua_State* L) -> int {
                auto name = luaL_checkstring(L, 1);
                auto type = dual::type_registry_t::get().get_type(name);
                if(type == dual::kInvalidSIndex)
                {
                    lua_pushnil(L);
                    return 1;
                }
                lua_pushinteger(L, type);
                return 1;
            };
            lua_pushcfunction(L, trampoline);
            lua_setfield(L, -2, "component");
        }

        //bind allocate entities
        {
            auto trampoline = +[](lua_State* L) -> int {
                dual_storage_t* storage = (dual_storage_t*)lua_touserdata(L, 1);
                luaL_argexpected(L, lua_istable(L, 2), 2, "table");
                luaL_argexpected(L, lua_isfunction(L, 3), 3, "function");
                auto size = luaL_checkinteger(L, 3);
                //iterate array
                auto count = lua_rawlen(L, 2);
                dual::type_builder_t builder;
                builder.reserve(count);
                for(auto i = 1; i <= count; ++i)
                {
                    lua_rawgeti(L, 2, i);
                    auto type = luaL_checkinteger(L, -1);
                    builder.with(type);
                    lua_pop(L, 1);
                }
                dual_entity_type_t type;
                type.type = builder.build();
                lua_pushlightuserdata(L, &type);
                auto callback = [&](dual_chunk_view_t* view) -> void {
                    dual::fixed_stack_scope_t scope(dual::localStack);
                    auto luaView = init_chunk_view(view, type);
                    lua_pushvalue(L, 3);
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
                dualS_allocate_type(storage, &type, size, DUAL_LAMBDA(callback));
                return 0;
            };
            lua_pushcfunction(L, trampoline);
            lua_setfield(L, -2, "allocate_entities");
        }

        // bind destroy entities
        {
            auto trampoline = +[](lua_State* L) -> int {
                dual_storage_t* storage = (dual_storage_t*)lua_touserdata(L, 1);
                luaL_argexpected(L, lua_istable(L, 2), 2, "table");
                //iterate array
                auto count = lua_rawlen(L, 2);
                eastl::vector<dual_entity_t> entities;
                entities.reserve(count);
                for(auto i = 1; i <= count; ++i)
                {
                    lua_rawgeti(L, 2, i);
                    auto ent = luaL_checkinteger(L, -1);
                    entities.push_back(dual_entity_t(ent));
                    lua_pop(L, 1);
                }
                dual_view_callback_t callback = +[](void* userdata, dual_chunk_view_t* view) -> void {
                    dualS_destroy((dual_storage_t*)userdata, view);
                };
                dualS_batch(storage, entities.data(), (uint32_t)entities.size(), callback, storage);
                return 0;
            };
            lua_pushcfunction(L, trampoline);
            lua_setfield(L, -2, "destroy_entities");
        }

        // bind cast entities
        {
            auto trampoline = +[](lua_State* L) -> int {
                dual_storage_t* storage = (dual_storage_t*)lua_touserdata(L, 1);
                luaL_argexpected(L, lua_istable(L, 2), 2, "table");
                luaL_argexpected(L, lua_istable(L, 3), 3, "table");
                bool withRemove = lua_toboolean(L, 4);
                luaL_argexpected(L, lua_isfunction(L, 4 + withRemove), 4 + withRemove, "table");
                //iterate array
                auto count = lua_rawlen(L, 2);
                eastl::vector<dual_entity_t> entities;
                dual::type_builder_t addBuilder;
                dual::type_builder_t removeBuilder;
                entities.reserve(count);
                for(auto i = 1; i <= count; ++i)
                {
                    lua_rawgeti(L, 2, i);
                    auto ent = luaL_checkinteger(L, -1);
                    entities.push_back(dual_entity_t(ent));
                    lua_pop(L, 1);
                }
                    auto addCount = lua_rawlen(L, 3);
                    addBuilder.reserve(addCount);
                    for(auto i = 1; i <= addCount; ++i)
                    {
                        lua_rawgeti(L, 3, i);
                        auto type = luaL_checkinteger(L, -1);
                        addBuilder.with(type);
                        lua_pop(L, 1);
                    }
                uint32_t param = 4;
                if(lua_istable(L, 4))
                {
                    auto removeCount = lua_rawlen(L, 4);
                    removeBuilder.reserve(removeCount);
                    for(auto i = 1; i <= removeCount; ++i)
                    {
                        lua_rawgeti(L, 4, i);
                        auto type = luaL_checkinteger(L, -1);
                        removeBuilder.with(type);
                        lua_pop(L, 1);
                    }
                    param = 5;
                }
                dual_delta_type_t delta;
                delta.added.type = addBuilder.build();
                delta.removed.type = removeBuilder.build();
                auto callback = [&](dual_chunk_view_t* view) -> void {
                    auto castCallback = [&](dual_chunk_view_t* new_view, dual_chunk_view_t* old_view)
                    {
                        dual::fixed_stack_scope_t scope(dual::localStack);
                        auto luaView = init_chunk_view(view, delta.added);
                        lua_pushvalue(L, param);
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
                    dualS_cast_view_delta(storage, view, &delta, DUAL_LAMBDA(castCallback));
                };
                dualS_batch(storage, entities.data(), (uint32_t)entities.size(), DUAL_LAMBDA(callback));
                return 0;
            };
            lua_pushcfunction(L, trampoline);
            lua_setfield(L, -2, "cast_entities");
        }

        // bind query
        {
            luaL_Reg metamethods[] = {
                {"__gc", +[](lua_State* L) -> int {
                    auto query = *(dual_query_t**)lua_touserdata(L, 1);
                    if(!query) return 0;
                    dualQ_release(query);
                    return 0;
                }},
                {NULL, NULL}
            };
            luaL_newmetatable(L, "dual_query_t");
            luaL_setfuncs(L, metamethods, 0);
            lua_pop(L, 1);
        }

        // bind create query
        {
            auto trampoline = +[](lua_State* L) -> int {
                dual_storage_t* storage = (dual_storage_t*)lua_touserdata(L, 1);
                const char* literal = luaL_checkstring(L, 2);
                auto query = dualQ_from_literal(storage, literal);
                *(dual_query_t**)lua_newuserdata(L, sizeof(void*)) = query;
                return 1;
            };
            lua_pushcfunction(L, trampoline);
            lua_setfield(L, -2, "create_query");
        }
        
        // bind iterate query
        {
            auto trampoline = +[](lua_State* L) -> int {
                auto query = *(dual_query_t**)luaL_checkudata(L, 1, "dual_query_t");
                if(!query) return 0;
                luaL_argexpected(L, lua_isfunction(L, 2), 2, "function");
                dual_view_callback_t callback = +[](void* userdata, dual_chunk_view_t* view) -> void {
                    lua_State* L = (lua_State*)userdata;
                    dual_query_t* query = *(dual_query_t**)lua_touserdata(L, 1);
                    dual::fixed_stack_scope_t scope(dual::localStack);
                    auto luaView = query_chunk_view(view, query);
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

        // bind query chunk view
        {
            luaL_Reg metamethods[] = {
                { "__index", +[](lua_State* L) -> int {
                    lua_chunk_view_t* view = *(lua_chunk_view_t**)luaL_checkudata(L, 1, "lua_chunk_view_t");
                    auto field = luaL_checkstring(L, 2);
                    
                    if(strcmp(field, "length") == 0)
                    {
                        lua_pushinteger(L, view->view.count);
                        return 1;
                    }
                    else 
                    {
                        return luaL_error(L, "invalid chunk view field '%s'", field);
                    }
                    return 0;
                } },
                {"__call", +[](lua_State* L) -> int {
                    lua_chunk_view_t* view = *(lua_chunk_view_t**)luaL_checkudata(L, 1, "lua_chunk_view_t");
                    uint32_t index = luaL_checkinteger(L, 2);
                    luaL_argexpected(L, index < view->view.count, 2, "index out of bounds");
                    lua_pushinteger(L, view->entities[index]);
                    forloop(i, 0, view->count)
                    {
                        auto data = view->datas[i];
                        if(data == nullptr)
                        {
                            lua_pushnil(L);
                        }
                        else
                        {
                            data = (char *)data + view->strides[i] * index;
                            *(void**)lua_newuserdata(L, sizeof(void*)) = data;
                            luaL_getmetatable(L, view->guidStrs[i]);
                            if(lua_isnil(L, -1))
                                luaL_getmetatable(L, "skr_opaque_t");
                            lua_setmetatable(L, -2);
                        }
                    }
                    return view->count + 1;
                } },
                { NULL, NULL }
            };
            luaL_newmetatable(L, "lua_chunk_view_t");
            luaL_setfuncs(L, metamethods, 0);
            lua_pop(L, 1);
        }
    }
}