#pragma once
#include "lua/bind_fwd.hpp"
#include "containers/string.hpp"
#include "containers/sptr.hpp"
#include "resource/resource_handle.h"
#include "misc/traits.hpp"
#include "misc/join.hpp"
extern "C" {
#include "lua.h"
#include "lualib.h"
}

namespace skr::lua
{
    RUNTIME_API int push_enum(lua_State* L, long long v);
    RUNTIME_API long long check_enum(lua_State* L, int index);
    RUNTIME_API long long opt_enum(lua_State* L, int index, long long def);
    RUNTIME_API int push_guid(lua_State* L, const skr_guid_t* guid);
    RUNTIME_API const skr_guid_t* check_guid(lua_State* L, int index);
    RUNTIME_API const skr_guid_t* opt_guid(lua_State* L, int index, const skr_guid_t* def);
    RUNTIME_API int push_string(lua_State* L, const skr::string& str);
    RUNTIME_API int push_string(lua_State* L, skr::string_view str);
    RUNTIME_API skr::string check_string(lua_State* L, int index);
    RUNTIME_API skr::string opt_string(lua_State* L, int index, const skr::string& def);
    RUNTIME_API int push_resource(lua_State* L, const skr_resource_handle_t* resource);
    RUNTIME_API const skr_resource_handle_t* check_resource(lua_State* L, int index);
    RUNTIME_API const skr_resource_handle_t* opt_resource(lua_State* L, int index, const skr_resource_handle_t* def);
    using copy_constructor_t = void(*)(void* dst, const void* src);
    using constructor_t = void(*)(void* dst);
    using destructor_t = void(*)(void* dst);
    RUNTIME_API int push_unknown(lua_State* L, void* value, std::string_view tid);
    RUNTIME_API int push_unknown_value(lua_State* L, const void* value, std::string_view tid, size_t size, copy_constructor_t copy_constructor, destructor_t destructor);
    RUNTIME_API void* check_unknown(lua_State* L, int index, std::string_view tid);
    RUNTIME_API int push_sptr(lua_State* L, const skr::SPtr<void>& value, std::string_view tid);
    RUNTIME_API skr::SPtr<void> check_sptr(lua_State* L, int index, std::string_view tid);
    RUNTIME_API int push_sobjectptr(lua_State* L, const skr::SObjectPtr<SInterface>& value, std::string_view tid);
    RUNTIME_API skr::SObjectPtr<SInterface> check_sobjectptr(lua_State* L, int index, std::string_view tid);
    // TODO: how should we handle math operations in lua?
    // RUNTIME_API int push_float2(lua_State* L, const skr_float2_t* float2);
    // RUNTIME_API skr_float2_t check_float2(lua_State* L, int index);
    // RUNTIME_API int push_float3(lua_State* L, const skr_float3_t* float3);
    // RUNTIME_API skr_float3_t check_float3(lua_State* L, int index);
    // RUNTIME_API int push_float4(lua_State* L, const skr_float4_t* float4);
    // RUNTIME_API skr_float4_t check_float4(lua_State* L, int index);

    template<class T, class=void>
    struct DefaultBindTrait;

    template<class T>
    struct DefaultBindTrait<T*, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::type::type_id<T>>>>
    {
        static int push(lua_State* L, T* value)
        {
            return push_unknown(L, value, skr::type::type_id<T>::str());
        }
        static T* check(lua_State* L, int index)
        {
            return (T*)check_unknown(L, index, skr::type::type_id<T>::str());
        }
    };

    template<class T>
    struct DefaultBindTrait<T, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::type::type_id<T>>>>
    {
        static int push(lua_State* L, const T& value)
        {
            static constexpr std::string_view prefix = "[unique]";
            static constexpr std::string_view tid = skr::type::type_id<T>::str();
            return push_unknown_value(L, value, join_v<prefix, tid>, sizeof(T), +[](void* dst, const void* src) {
                new (dst) T(*(const T*)src);
            }, +[](void* dst) {
                ((T*)dst)->~T();
            });
        }
        static T& check(lua_State* L, int index)
        {
            return *(T*)check_unknown(L, index, skr::type::type_id<T>::str());
        }
    };

    template<class T>
    struct DefaultBindTrait<skr::SPtr<T>, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::type::type_id<T>>>>
    {
        static int push(lua_State* L, const skr::SPtr<T>& value)
        {
            static constexpr std::string_view prefix = "[shared]";
            static constexpr std::string_view tid = skr::type::type_id<T>::str();
            return push_sptr(L, value, join_v<prefix, tid>);
        }
        static skr::SPtr<T> check(lua_State* L, int index)
        {
            return reinterpret_pointer_cast<T>(check_sptr(L, index, skr::type::type_id<T>::get()));
        }
    };

    template<class T>
    struct DefaultBindTrait<skr::SObjectPtr<T>, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::type::type_id<T>>>>
    {
        static int push(lua_State* L, const skr::SObjectPtr<T>& value)
        {
            static constexpr std::string_view prefix = "[shared]";
            static constexpr std::string_view tid = skr::type::type_id<T>::str();
            return push_sobjectptr(L, value, join_v<prefix, tid>);
        }
        static skr::SObjectPtr<T> check(lua_State* L, int index)
        {
            return reinterpret_pointer_cast<T>(check_sobjectptr(L, index, skr::type::type_id<T>::get()));
        }
    };

    template <class T>
    struct DefaultBindTrait<T, std::enable_if_t<std::is_enum_v<T>>>
    {
        using type = T;
        static int push(lua_State* L, type value)
        {
            push_enum(L, static_cast<long long>(value));
            return 1;
        }
        static type check(lua_State* L, int index)
        {
            return static_cast<type>(check_enum(L, index));
        }
        static type opt(lua_State* L, int index, type def)
        {
            return static_cast<type>(opt_enum(L, index, static_cast<long long>(def)));
        }
    };
    
    template<class T>
    struct DefaultBindTrait<const T&, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::type::type_id<T>>>>
    {
        static int push(lua_State* L, const T& value)
        {
            return DefaultBindTrait<T>::push(L, value);
        }
        static const T& check(lua_State* L, int index)
        {
            return DefaultBindTrait<T>::check(L, index);
        }
    };

    template<class T>
    int push(lua_State* L, const T& value)
    {
        using TT = std::remove_const_t<std::remove_reference_t<T>>;
        if constexpr(skr::is_complete_v<BindTrait<T>>)
            return BindTrait<TT>::push(L, value);
        else
            return DefaultBindTrait<TT>::push(L, value);
    }

    template<class T>
    decltype(auto) check(lua_State* L, int index, int& used)
    {
        using TT = std::remove_const_t<std::remove_reference_t<T>>;
        if constexpr(skr::is_complete_v<BindTrait<T>>)
            return BindTrait<TT>::check(L, index, used);
        else
        {
            used = 1;
            return DefaultBindTrait<TT>::check(L, index);
        }
    }

    struct shared_userdata_t
    {
        void* data;
        SPtr<void> shared;
    };

    template<>
    struct BindTrait<float>
    {
        static int push(lua_State* L, float value)
        {
            lua_pushnumber(L, value);
            return 1;
        }
        static float check(lua_State* L, int index, int& used)
        {
            used = 1;
            return (float)luaL_checknumber(L, index);
        }
        static float opt(lua_State* L, int index, float def)
        {
            return (float)luaL_optnumber(L, index, def);
        }
    };

    template<>
    struct BindTrait<double>
    {
        static int push(lua_State* L, double value)
        {
            lua_pushnumber(L, value);
            return 1;
        }
        static double check(lua_State* L, int index, int& used)
        {
            used = 1;
            return luaL_checknumber(L, index);
        }
        static double opt(lua_State* L, int index, double def)
        {
            return luaL_optnumber(L, index, def);
        }
    };

    #define BindInt(type) \
    template<> \
    struct BindTrait<type> \
    { \
        static int push(lua_State* L, type value) \
        { \
            lua_pushinteger(L, (type)value); \
            return 1; \
        } \
        static type check(lua_State* L, int index, int& used) \
        { \
            used = 1; \
            return (type)luaL_checkinteger(L, index); \
        } \
        static type opt(lua_State* L, int index, type def, int& used) \
        { \
            used = 1; \
            return (type)luaL_optinteger(L, index, (type)def); \
        } \
    };
    BindInt(uint32_t);
    BindInt(int32_t);
    BindInt(uint64_t);
    BindInt(int64_t);
    #undef BindInt
    
    template<>
    struct BindTrait<bool>
    {
        static int push(lua_State* L, bool value)
        {
            lua_pushboolean(L, value);
            return 1;
        }
        static bool check(lua_State* L, int index, int& used)
        {
            used = 1;
            return lua_toboolean(L, index) != 0;
        }
        static bool opt(lua_State* L, int index, bool def)
        {
            return lua_isnoneornil(L, index) ? def : lua_toboolean(L, index) != 0;
        }
    };

    template<>
    struct BindTrait<skr_guid_t>
    {
        static int push(lua_State* L, const skr_guid_t& guid)
        {
            return push_guid(L, &guid);
        }

        static const skr_guid_t check(lua_State* L, int index, int& used)
        {
            used = 1;
            return *check_guid(L, index);
        }

        static const skr_guid_t opt(lua_State* L, int index, const skr_guid_t& def)
        {
            return *opt_guid(L, index, &def);
        }
    };

    template<>
    struct BindTrait<skr_resource_handle_t>
    {
        static int push(lua_State* L, const skr_resource_handle_t& resource)
        {
            return push_resource(L, &resource);
        }

        static const skr_resource_handle_t& check(lua_State* L, int index, int& used)
        {
            used = 1;
            return *check_resource(L, index);
        }

        static const skr_resource_handle_t& opt(lua_State* L, int index, const skr_resource_handle_t& def)
        {
            return *opt_resource(L, index, &def);
        }
    };

    template<class T>
    struct BindTrait<resource::TResourceHandle<T>>
    {
        static int push(lua_State* L, const resource::TResourceHandle<T>& resource)
        {
            return push_resource(L, (const skr_resource_handle_t*)&resource);
        }

        static const resource::TResourceHandle<T>& check(lua_State* L, int index, int& used)
        {
            used = 1;
            return *(resource::TResourceHandle<T>*)check_resource(L, index);
        }

        static const resource::TResourceHandle<T>& opt(lua_State* L, int index, const resource::TResourceHandle<T>& def)
        {
            return *(resource::TResourceHandle<T>*)opt_resource(L, index, &def);
        }
    };

    template<>
    struct BindTrait<skr::string>
    {
        static int push(lua_State* L, const skr::string& str)
        {
            return push_string(L, str);
        }

        static skr::string check(lua_State* L, int index, int& used)
        {
            used = 1;
            return check_string(L, index);
        }

        static skr::string opt(lua_State* L, int index, const skr::string& def)
        {
            return opt_string(L, index, def);
        }
    };

    template<>
    struct BindTrait<skr::string_view>
    {
        static int push(lua_State* L, skr::string_view str)
        {
            return push_string(L, str);
        }
    };

    template<class T>
    struct RefParam;

    template<class T>
    struct RefParam<T&>
    {
        using type = T;
        template<class Y>
        static Y& ref(Y& value)
        {
            return value;
        }
    };

    template<class T>
    struct RefParam<T*>
    {
        using type = T;
        template<class Y>
        static Y* ref(Y& value)
        {
            return &value;
        }
    };

    template<class T>
    using refed_t = typename RefParam<T>::type;

    template<class T, class Y>
    decltype(auto) ref(Y& value)
    {
        return RefParam<T>::ref(value);
    }

    template<class T>
    decltype(auto) deref(T value)
    {
        if constexpr(std::is_reference_v<std::remove_const_t<T>>)
            return value;
        else
            return *value;
    }

    template<class T>
    struct SharedUserdata
    {
        T* data;
        using shared_t = std::conditional_t<is_object_v<T>, SObjectPtr<T>, SPtr<T>>;
        shared_t shared;
        SharedUserdata(shared_t shared)
            : data(shared.get())
            , shared(std::move(shared))
        {
        }
    };

    template<class ...Ts>
    void Call(lua_State* L, Ts&&... args)
    {
        auto oldTop = lua_gettop(L);
        int n[] = {BindTrait<Ts>::Push(L, std::forward<Ts>(args))...};
        int nargs = 0;
        for (int i = 0; i < sizeof...(Ts); ++i)
            nargs += n[i];
        if(lua_pcall(L, nargs, 0, 0) != LUA_OK)
        {
            lua_getglobal(L, "skr");
            lua_getfield(L, -1, "log_error");
            lua_pushvalue(L, -3);
            lua_call(L, 1, 0);
            lua_pop(L, 2);
        }
        lua_settop(L, oldTop);
    }

    template<class R, class ...Ts>
    R CallR(lua_State* L, Ts&&... args)
    {
        auto oldTop = lua_gettop(L);
        int n[] = {BindTrait<Ts>::Push(L, std::forward<Ts>(args))...};
        int nargs = 0;
        for (int i = 0; i < sizeof...(Ts); ++i)
            nargs += n[i];
        if(lua_pcall(L, nargs, LUA_MULTRET, 0) != LUA_OK)
        {
            lua_getglobal(L, "skr");
            lua_getfield(L, -1, "log_error");
            lua_pushvalue(L, -3);
            lua_call(L, 1, 0);
            lua_pop(L, 2);
        }
        R ret = BindTrait<R>::Check(L, -1);
        lua_settop(L, oldTop);
    }
}