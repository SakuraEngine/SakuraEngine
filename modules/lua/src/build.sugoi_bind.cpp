extern "C" {
#include "lua.h"
#include "lualib.h"
}
#include "SkrRT/ecs/detail/stack.hpp"
#include "SkrRT/ecs/detail/query.hpp"
#include "SkrRT/ecs/detail/chunk.hpp"
#include "SkrRT/ecs/detail/archetype.hpp"
#include "SkrRT/ecs/detail/type_registry.hpp"
#include "SkrRT/ecs/type_builder.hpp"

#include "SkrRT/ecs/array.hpp"

namespace skr::lua
{
    thread_local sugoi::fixed_stack_t localStack(4096 * 8);

    using lua_push_t = int (*)(sugoi_chunk_t* chunk, EIndex index, char* data, struct lua_State* L);
    using lua_check_t = void (*)(sugoi_chunk_t* chunk, EIndex index, char* data, struct lua_State* L, int idx);

    struct lua_array_view_t {
        sugoi_chunk_view_t view;
        uint32_t index;
        sugoi_array_comp_t arr;
        uint32_t stride;
        const char8_t* guidStr;
        lua_push_t lua_push;
        lua_check_t lua_check;
    };

    struct lua_chunk_view_t {
        sugoi_storage_t* storage;
        sugoi_chunk_view_t view;
        uint32_t count;
        bool readonly;
        const sugoi_entity_t* entities;
        const sugoi_type_index_t* types;
        void** datas;
        uint32_t* strides;
        const char8_t** guidStrs;
        uint32_t* elementSizes;
        lua_push_t* lua_pushs;
        lua_check_t* lua_checks;
        const sugoi_operation_t* operations;
        int pushComponent(lua_State* L, int comp, int index)
        {
            auto data = datas[comp];
            if(data == nullptr)
            {
                lua_pushnil(L);
                return 1;
            }
            else if(elementSizes[comp] != 0)
            {
                auto arr = (lua_array_view_t*)lua_newuserdata(L, sizeof(lua_array_view_t));
                arr->arr = *(sugoi_array_comp_t*)((uint8_t*)data + index * strides[comp]);
                arr->view = view;
                arr->index = index;
                arr->stride = elementSizes[comp];
                arr->guidStr = guidStrs[comp];
                arr->lua_push = lua_pushs[comp];
                arr->lua_check = lua_checks[comp];
                luaL_getmetatable(L, "lua_array_view_t");
                lua_setmetatable(L, -2);
                return 1;
            }
            else if(auto push = lua_pushs[comp])
            {
                return push(view.chunk, view.start+index, (char *)data + strides[comp] * index, L);
            }
            else
            {
                data = (char *)data + strides[comp] * index;
                *(void**)lua_newuserdata(L, sizeof(void*)) = data;
                luaL_getmetatable(L, (const char*)guidStrs[comp]);
                if(lua_isnil(L, -1))
                    luaL_getmetatable(L, "skr_opaque_t");
                lua_setmetatable(L, -2);
                return 1;
            }
        };
    };

    static lua_chunk_view_t inherit_chunk_view(sugoi_chunk_view_t* view, lua_chunk_view_t* parent)
    {
        lua_chunk_view_t luaView { parent->storage, *view, parent->count, parent->readonly, parent->entities, parent->types, parent->datas, parent->strides, parent->guidStrs, parent->elementSizes, parent->lua_pushs, parent->lua_checks };
        luaView.datas = localStack.allocate<void*>(parent->count);
        luaView.entities = sugoiV_get_entities(view);
        forloop(i, 0, parent->count)
        {
            if((luaView.operations && !luaView.operations[i].readonly) || luaView.readonly)
                luaView.datas[i] = sugoiV_get_owned_rw(view, luaView.types[i]);
            else
                luaView.datas[i] = (void*)sugoiV_get_owned_ro(view, luaView.types[i]);
        }
        return luaView;
    }

    static lua_chunk_view_t fill_chunk_view(sugoi_storage_t* storage, sugoi_chunk_view_t* view, const sugoi_type_index_t* indices, uint32_t count, const sugoi_operation_t* operations, bool readonly)
    {
        lua_chunk_view_t luaView { storage, *view, count };

        luaView.types = indices;
        luaView.datas = localStack.allocate<void*>(count);
        luaView.strides = localStack.allocate<uint32_t>(count);
        luaView.guidStrs = localStack.allocate<const char8_t*>(count);
        luaView.elementSizes = localStack.allocate<uint32_t>(count);
        luaView.lua_pushs = localStack.allocate<lua_push_t>(count);
        luaView.lua_checks = localStack.allocate<lua_check_t>(count);
        luaView.operations = operations;
        luaView.readonly = readonly;

        luaView.entities = sugoiV_get_entities(view);
        auto& typeReg = sugoi::type_registry_t::get();
        forloop(i, 0, count)
        {
            auto& desc = typeReg.descriptions[sugoi::type_index_t(indices[i]).index()];
            if((operations && !operations[i].readonly) || !readonly)
            {
                luaView.lua_checks[i] = desc.callback.lua_check;
                luaView.datas[i] = (void*)sugoiV_get_owned_rw(view, indices[i]);
            }
            else
            {
                luaView.lua_checks[i] = nullptr;
                luaView.datas[i] = (void*)sugoiV_get_owned_ro(view, indices[i]);
            }
            luaView.strides[i] = desc.size;
            luaView.guidStrs[i] = desc.guidStr;
            luaView.elementSizes[i] = desc.elementSize;
            luaView.lua_pushs[i] = desc.callback.lua_push;
        }
        return luaView;
    }

    static lua_chunk_view_t init_chunk_view(sugoi_storage_t* storage, sugoi_chunk_view_t* view, const sugoi_type_set_t& type, bool readonly = true)
    {
        return fill_chunk_view(storage, view, type.data, type.length, nullptr, readonly);
    }

    static lua_chunk_view_t query_chunk_view(sugoi_chunk_view_t* view, sugoi_query_t* query)
    {
        return fill_chunk_view(query->storage, view, query->parameters.types, query->parameters.length, query->parameters.accesses, true);
    }

    void dtor_query(void* p)
    {
        auto query = (sugoi_query_t*)p;
        sugoiQ_release(query);
    }

    void bind_ecs(lua_State* L)
    {
        lua_getglobal(L, "skr");

        //bind component
        {
            auto trampoline = +[](lua_State* L) -> int {
                auto name = (const char8_t*)luaL_checkstring(L, 1);
                auto type = sugoi::type_registry_t::get().get_type(name);
                if(type == sugoi::kInvalidSIndex)
                {
                    lua_pushnil(L);
                    return 1;
                }
                lua_pushinteger(L, type);
                return 1;
            };
            lua_pushcfunction(L, trampoline, "component");
            lua_setfield(L, -2, "component");
        }

        // bind batch
        {
        }

        //bind allocate entities
        {
            auto trampoline = +[](lua_State* L) -> int {
                sugoi_storage_t* storage = (sugoi_storage_t*)lua_touserdata(L, 1);
                luaL_argexpected(L, lua_istable(L, 2), 2, "table");
                auto size = luaL_checkinteger(L, 3);
                luaL_argexpected(L, lua_isfunction(L, 4), 4, "function");
                //iterate array
                auto count = lua_objlen(L, 2);
                sugoi::type_builder_t builder;
                builder.reserve((uint32_t)count);
                for(auto i = 1; i <= count; ++i)
                {
                    lua_rawgeti(L, 2, i);
                    auto type = luaL_checkinteger(L, -1);
                    builder.with((sugoi_type_index_t)type);
                    lua_pop(L, 1);
                }
                sugoi_entity_type_t type;
                type.type = builder.build();
                auto callback = [&](sugoi_chunk_view_t* view) -> void {
                    sugoi::fixed_stack_scope_t scope(localStack);
                    auto luaView = init_chunk_view(storage, view, type.type);
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
                sugoiS_allocate_type(storage, &type, (EIndex)size, SUGOI_LAMBDA(callback));
                return 0;
            };
            lua_pushcfunction(L, trampoline, "allocate_entities");
            lua_setfield(L, -2, "allocate_entities");
        }

        // bind destroy entities
        {
            auto trampoline = +[](lua_State* L) -> int {
                sugoi_storage_t* storage = (sugoi_storage_t*)lua_touserdata(L, 1);
                luaL_argexpected(L, lua_istable(L, 2), 2, "table");
                //iterate array
                auto count = lua_objlen(L, 2);
                skr::stl_vector<sugoi_entity_t> entities;
                entities.reserve(count);
                for(auto i = 1; i <= count; ++i)
                {
                    lua_rawgeti(L, 2, i);
                    auto ent = luaL_checkinteger(L, -1);
                    entities.push_back(sugoi_entity_t(ent));
                    lua_pop(L, 1);
                }
                sugoi_view_callback_t callback = +[](void* userdata, sugoi_chunk_view_t* view) -> void {
                    sugoiS_destroy((sugoi_storage_t*)userdata, view);
                };
                sugoiS_batch(storage, entities.data(), (uint32_t)entities.size(), callback, storage);
                return 0;
            };
            lua_pushcfunction(L, trampoline, "destroy_entities");
            lua_setfield(L, -2, "destroy_entities");
        }

        // bind cast entities
        {
            auto trampoline = +[](lua_State* L) -> int {
                sugoi_storage_t* storage = (sugoi_storage_t*)lua_touserdata(L, 1);
                luaL_argexpected(L, lua_istable(L, 2), 2, "table");
                luaL_argexpected(L, lua_istable(L, 3), 3, "table");
                bool withRemove = lua_toboolean(L, 4);
                luaL_argexpected(L, lua_isfunction(L, 4 + withRemove), 4 + withRemove, "table");
                //iterate array
                auto count = lua_objlen(L, 2);
                skr::stl_vector<sugoi_entity_t> entities;
                sugoi::type_builder_t addBuilder;
                sugoi::type_builder_t removeBuilder;
                entities.reserve(count);
                for(auto i = 1; i <= count; ++i)
                {
                    lua_rawgeti(L, 2, i);
                    auto ent = luaL_checkinteger(L, -1);
                    entities.push_back(sugoi_entity_t(ent));
                    lua_pop(L, 1);
                }
                    auto addCount = lua_objlen(L, 3);
                    addBuilder.reserve((uint32_t)addCount);
                    for(auto i = 1; i <= addCount; ++i)
                    {
                        lua_rawgeti(L, 3, i);
                        auto type = luaL_checkinteger(L, -1);
                        addBuilder.with((sugoi_type_index_t)type);
                        lua_pop(L, 1);
                    }
                uint32_t param = 4;
                if(lua_istable(L, 4))
                {
                    auto removeCount = lua_objlen(L, 4);
                    removeBuilder.reserve((uint32_t)removeCount);
                    for(auto i = 1; i <= removeCount; ++i)
                    {
                        lua_rawgeti(L, 4, i);
                        auto type = luaL_checkinteger(L, -1);
                        removeBuilder.with((sugoi_type_index_t)type);
                        lua_pop(L, 1);
                    }
                    param = 5;
                }
                sugoi_delta_type_t delta;
                delta.added.type = addBuilder.build();
                delta.removed.type = removeBuilder.build();
                auto callback = [&](sugoi_chunk_view_t* view) -> void {
                    auto castCallback = [&](sugoi_chunk_view_t* new_view, sugoi_chunk_view_t* old_view)
                    {
                        sugoi::fixed_stack_scope_t scope(localStack);
                        auto luaView = init_chunk_view(storage, view, delta.added.type);
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
                    sugoiS_cast_view_delta(storage, view, &delta, SUGOI_LAMBDA(castCallback));
                };
                sugoiS_batch(storage, entities.data(), (uint32_t)entities.size(), SUGOI_LAMBDA(callback));
                return 0;
            };
            lua_pushcfunction(L, trampoline, "cast_entities");
            lua_setfield(L, -2, "cast_entities");
        }

        // bind query
        {
            luaL_newmetatable(L, "sugoi_query_t");
            lua_pop(L, 1);
        }

        // bind create query
        {
            auto trampoline = +[](lua_State* L) -> int {
                sugoi_storage_t* storage = (sugoi_storage_t*)lua_touserdata(L, 1);
                const char8_t* literal = (const char8_t*)luaL_checkstring(L, 2);
                auto query = sugoiQ_from_literal(storage, literal);
                *(sugoi_query_t**)lua_newuserdatadtor(L, sizeof(void*), dtor_query) = query;
                luaL_getmetatable(L, "sugoi_query_t");
                lua_setmetatable(L, -2);
                return 1;
            };
            lua_pushcfunction(L, trampoline, "create_query");
            lua_setfield(L, -2, "create_query");
        }
        
        // bind iterate query
        {
            auto trampoline = +[](lua_State* L) -> int {
                auto query = *(sugoi_query_t**)luaL_checkudata(L, 1, "sugoi_query_t");
                if(!query) return 0;
                luaL_argexpected(L, lua_isfunction(L, 2), 2, "function");
                sugoi_view_callback_t callback = +[](void* userdata, sugoi_chunk_view_t* view) -> void {
                    lua_State* L = (lua_State*)userdata;
                    sugoi_query_t* query = *(sugoi_query_t**)lua_touserdata(L, 1);
                    sugoi::fixed_stack_scope_t scope(localStack);
                    auto luaView = query_chunk_view(view, query);
                    lua_pushvalue(L, 2);

                    auto ptr = (lua_chunk_view_t**)lua_newuserdata(L, sizeof(void*));
                    *ptr = &luaView;
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
                    *ptr = nullptr;
                };
                void* u = (void*)L;
                sugoiQ_get_views(query, callback, u);
                return 0;
            };
            lua_pushcfunction(L, trampoline, "iterate_query");
            lua_setfield(L, -2, "iterate_query");
        }
        lua_pop(L, 1);

        // bind lua chunk view
        {
            luaL_Reg metamethods[] = {
                { "__index", +[](lua_State* L) -> int {
                    lua_chunk_view_t* view = *(lua_chunk_view_t**)luaL_checkudata(L, 1, "lua_chunk_view_t");
                    if(!view) 
                    {
                        luaL_error(L, "chunk view cannot be accessed after query iteration");
                        return 0;
                    }
                    auto field = luaL_checkstring(L, 2);
                    
                    if(strcmp(field, "length") == 0)
                    {
                        lua_pushinteger(L, view->view.count);
                        return 1;
                    }
                    else if(strcmp(field, "set") == 0)
                    {
                        lua_pushcfunction(L, +[](lua_State* L) -> int
                        {
                            lua_chunk_view_t* view = (lua_chunk_view_t*)luaL_checkudata(L, 1, "lua_chunk_view_t");
                            int index = (int)luaL_checkinteger(L, 2);
                            int compId = 0;
                            luaL_argexpected(L, lua_isstring(L, 3) || lua_isnumber(L, 3), 3, "expected name or localindex");
                            if(lua_isstring(L, 3))
                            {
                                auto str = (const char8_t*)lua_tostring(L, 3);
                                auto id = sugoiT_get_type_by_name(str);
                                compId = view->view.chunk->type->index(id);
                            }
                            else if(lua_isnumber(L, 3))
                            {
                                compId = (int)luaL_checkinteger(L, 3);
                            }
                            luaL_argexpected(L, index < (int)view->view.count, 2, "index out of bounds");
                            luaL_argexpected(L, index < (int)view->count, 3, "index out of bounds");
                            if(view->elementSizes[compId] != 0)
                            {
                                luaL_error(L, "array component is not direct writable %s", view->guidStrs[compId]);
                                return 0;
                            }
                            auto check = view->lua_checks[compId];
                            if(!check)
                            {
                                luaL_error(L, "component is not direct writable %s", view->guidStrs[compId]);
                                return 0;
                            }
                            auto data = view->datas[compId];
                            data = (uint8_t*)data + index * view->strides[compId];
                            check(view->view.chunk, view->view.start + index, (char*)data, L, 4);
                            return 1;
                        }, "set");
                        return 1;
                    }
                    else if(strcmp(field, "entity") == 0)
                    {
                        lua_pushcfunction(L, +[](lua_State* L) -> int
                        {
                            lua_chunk_view_t* view = (lua_chunk_view_t*)luaL_checkudata(L, 1, "lua_chunk_view_t");
                            int index = (int)luaL_checkinteger(L, 2);
                            luaL_argexpected(L, index < (int)view->view.count, 2, "index out of bounds");
                            luaL_argexpected(L, index < (int)view->count, 3, "index out of bounds");
                            auto entity = view->entities[index];
                            lua_pushinteger(L, entity);
                            return 1;
                        }, "entity");
                        return 1;
                    }
                    else if(strcmp(field, "get") == 0)
                    {
                        lua_pushcfunction(L, +[](lua_State* L) -> int
                        {
                            lua_chunk_view_t* view = (lua_chunk_view_t*)luaL_checkudata(L, 1, "lua_chunk_view_t");
                            int index = (int)luaL_checkinteger(L, 2);
                            int compId = 0;
                            luaL_argexpected(L, lua_isstring(L, 3) || lua_isnumber(L, 3), 3, "expected name or localindex");
                            if(lua_isstring(L, 3))
                            {
                                auto str = (const char8_t*)lua_tostring(L, 3);
                                auto id = sugoiT_get_type_by_name(str);
                                compId = view->view.chunk->type->index(id);
                            }
                            else if(lua_isnumber(L, 3))
                            {
                                compId = (int)luaL_checkinteger(L, 3);
                            }
                            return view->pushComponent(L, compId, index);
                        }, "get");
                        return 1;
                    }
                    else if(strcmp(field, "unpack") == 0)
                    {
                        lua_pushcfunction(L, +[](lua_State* L) -> int
                        {
                            lua_chunk_view_t* view = *(lua_chunk_view_t**)luaL_checkudata(L, 1, "lua_chunk_view_t");
                            if(!view) 
                            {
                                luaL_error(L, "chunk view cannot be accessed after query iteration");
                                return 0;
                            }
                            uint32_t index = (uint32_t)luaL_checkinteger(L, 2);
                            luaL_argexpected(L, index < view->view.count, 2, "index out of bounds");
                            lua_pushinteger(L, view->entities[index]);
                            uint32_t ret = 1;
                            forloop(i, 0, view->count)
                                ret+=view->pushComponent(L, i, index);
                            return ret;
                        }, "unpack");
                        return 1;
                    }
                    else if(strcmp(field, "with") == 0)
                    {
                        auto trampoline = +[](lua_State* L) -> int {
                            lua_chunk_view_t* parent = *(lua_chunk_view_t**)luaL_checkudata(L, 1, "lua_chunk_view_t");
                            luaL_argexpected(L, lua_istable(L, 2), 2, "table");
                            luaL_argexpected(L, lua_isfunction(L, 3), 3, "function");
                            //iterate array
                            auto count = lua_objlen(L, 2);
                            skr::stl_vector<sugoi_entity_t> entities;
                            entities.reserve(count);
                            for(auto i = 1; i <= count; ++i)
                            {
                                lua_rawgeti(L, 2, i);
                                auto ent = luaL_checkinteger(L, -1);
                                entities.push_back(sugoi_entity_t(ent));
                                lua_pop(L, 1);
                            }
                            auto callback = [&](sugoi_chunk_view_t* view) -> void {
                                sugoi::fixed_stack_scope_t scope(localStack);
                                auto luaView = inherit_chunk_view(view, parent);
                                lua_pushvalue(L, 3);
                                auto ptr = (lua_chunk_view_t**)lua_newuserdata(L, sizeof(void*));
                                *ptr = &luaView;
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
                                *ptr = nullptr;
                            };
                            sugoiS_batch(parent->storage, entities.data(), (uint32_t)entities.size(), SUGOI_LAMBDA(callback));
                            return 0;
                        };
                        lua_pushcfunction(L, trampoline, "with");
                        return 1;
                    }
                    else 
                    {
                        luaL_error(L, "invalid chunk view field '%s'", field);
                        return 0;
                    }
                    return 0;
                } },
                { NULL, NULL }
            };
            luaL_newmetatable(L, "lua_chunk_view_t");
            luaL_register(L, nullptr, metamethods);
            lua_pop(L, 1);
        }

        //bind lua array view
        {
            luaL_Reg metamethods[] = {
                { "__index", +[](lua_State* L) -> int {
                    lua_array_view_t* view = (lua_array_view_t*)luaL_checkudata(L, 1, "lua_array_view_t");
                    auto field = luaL_checkstring(L, 2);
                    
                    if(strcmp(field, "length") == 0)
                    {
                        auto size = ((uint8_t*)view->arr.EndX - (uint8_t*)view->arr.BeginX) / view->stride;
                        lua_pushinteger(L, (int)size);
                        return 1;
                    }
                    else if(strcmp(field, "get") == 0)
                    {
                        auto trampoline = +[](lua_State* L) -> int {
                            lua_array_view_t* view = (lua_array_view_t*)luaL_checkudata(L, 1, "lua_array_view_t");
                            uint32_t index = (uint32_t)luaL_checkinteger(L, 2);
                            auto size = ((uint8_t*)view->arr.EndX - (uint8_t*)view->arr.BeginX) / view->stride;
                            luaL_argexpected(L, index < size, 2, "index out of bounds");
                            if(view->lua_push)
                            {
                                return view->lua_push(view->view.chunk, view->view.start+view->index, (char *)view->arr.BeginX + view->stride * index, L);
                            }
                            else
                            {
                                auto data = (char *)view->arr.BeginX + view->stride * index;
                                *(void**)lua_newuserdata(L, sizeof(void*)) = data;
                                luaL_getmetatable(L, (const char*)view->guidStr);
                                if(lua_isnil(L, -1))
                                    luaL_getmetatable(L, "skr_opaque_t");
                                lua_setmetatable(L, -2);
                                return 1;
                            }
                        };
                        lua_pushcfunction(L, trampoline, "get");
                        return 1;
                    }
                    else 
                    {
                        luaL_error(L, "invalid array view field '%s'", field);
                        return 0;
                    }
                    return 0;
                }},
                { NULL, NULL }
            };
            luaL_newmetatable(L, "lua_array_view_t");
            luaL_register(L, nullptr, metamethods);
            lua_pop(L, 1);
        }
    }
}