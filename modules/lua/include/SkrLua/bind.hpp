#pragma once
#include "SkrBase/misc/traits.hpp"
#include "SkrBase/misc/constexpr_join.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/sptr.hpp"
#include "SkrRT/resource/resource_handle.h"
#include "SkrRT/rttr/rttr_traits.hpp"
#include "SkrBase/config.h"
#include "SkrLua/bind_fwd.hpp"

extern "C" {
#include "lua.h"
#include "lualib.h"
}

namespace skr::lua
{
SKR_LUA_API int               push_enum(lua_State* L, long long v);
SKR_LUA_API long long         check_enum(lua_State* L, int index);
SKR_LUA_API long long         opt_enum(lua_State* L, int index, long long def);
SKR_LUA_API int               push_guid(lua_State* L, const skr_guid_t* guid);
SKR_LUA_API const skr_guid_t* check_guid(lua_State* L, int index);
SKR_LUA_API const skr_guid_t* opt_guid(lua_State* L, int index, const skr_guid_t* def);
SKR_LUA_API int               push_string(lua_State* L, const skr::String& str);
SKR_LUA_API int               push_string(lua_State* L, skr::StringView str);
SKR_LUA_API skr::String check_string(lua_State* L, int index);
SKR_LUA_API skr::String                  opt_string(lua_State* L, int index, const skr::String& def);
SKR_LUA_API int                          push_resource(lua_State* L, const skr_resource_handle_t* resource);
SKR_LUA_API const skr_resource_handle_t* check_resource(lua_State* L, int index);
SKR_LUA_API const skr_resource_handle_t* opt_resource(lua_State* L, int index, const skr_resource_handle_t* def);
using copy_constructor_t = void              (*)(void* dst, const void* src);
using constructor_t      = void                   (*)(void* dst);
using destructor_t       = void                    (*)(void* dst);
SKR_LUA_API int                          push_unknown(lua_State* L, void* value, std::string_view tid);
SKR_LUA_API int                          push_unknown_value(lua_State* L, const void* value, std::string_view tid, size_t size, copy_constructor_t copy_constructor, destructor_t destructor);
SKR_LUA_API void*                        check_unknown(lua_State* L, int index, std::string_view tid);
SKR_LUA_API int                          push_sptr(lua_State* L, const skr::SPtr<void>& value, std::string_view tid);
SKR_LUA_API skr::SPtr<void> check_sptr(lua_State* L, int index, std::string_view tid);
SKR_LUA_API int             push_sobjectptr(lua_State* L, const skr::SObjectPtr<SInterface>& value, std::string_view tid);
SKR_LUA_API skr::SObjectPtr<SInterface> check_sobjectptr(lua_State* L, int index, std::string_view tid);
// TODO: how should we handle math operations in lua?
// SKR_LUA_API int push_float2(lua_State* L, const skr_float2_t* float2);
// SKR_LUA_API skr_float2_t check_float2(lua_State* L, int index);
// SKR_LUA_API int push_float3(lua_State* L, const skr_float3_t* float3);
// SKR_LUA_API skr_float3_t check_float3(lua_State* L, int index);
// SKR_LUA_API int push_float4(lua_State* L, const skr_float4_t* float4);
// SKR_LUA_API skr_float4_t check_float4(lua_State* L, int index);

template <class T, class = void>
struct DefaultBindTrait;

template <class T>
struct DefaultBindTrait<T*, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::rttr::RTTRTraits<T>>>> {
    static int push(lua_State* L, T* value)
    {
        return push_unknown(L, value, skr::rttr::type_name<T>());
    }
    static T* check(lua_State* L, int index)
    {
        return (T*)check_unknown(L, index, skr::rttr::type_name<T>());
    }
};

template <class T>
struct DefaultBindTrait<T, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::rttr::RTTRTraits<T>>>> {
    static int push(lua_State* L, const T& value)
    {
        static constexpr std::string_view prefix = "[unique]";
        static constexpr std::string_view tid    = skr::rttr::type_name<T>();
        return push_unknown_value(
        L, value, constexpr_join_v<prefix, tid>, sizeof(T), +[](void* dst, const void* src) { new (dst) T(*(const T*)src); }, +[](void* dst) { ((T*)dst)->~T(); });
    }
    static T& check(lua_State* L, int index)
    {
        ::skr::String type_name = skr::rttr::type_name<T>();
        return *(T*)check_unknown(L, index, { type_name.c_str(), type_name.size() });
    }
};

template <class T>
struct DefaultBindTrait<skr::SPtr<T>, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::rttr::RTTRTraits<T>>>> {
    static int push(lua_State* L, const skr::SPtr<T>& value)
    {
        static constexpr std::string_view prefix = "[shared]";
        static constexpr std::string_view tid    = skr::rttr::type_name<T>();
        return push_sptr(L, value, constexpr_join_v<prefix, tid>);
    }
    static skr::SPtr<T> check(lua_State* L, int index)
    {
        return reinterpret_pointer_cast<T>(check_sptr(L, index, ::skr::rttr::type_id<T>()));
    }
};

template <class T>
struct DefaultBindTrait<skr::SObjectPtr<T>, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::rttr::RTTRTraits<T>>>> {
    static int push(lua_State* L, const skr::SObjectPtr<T>& value)
    {
        static constexpr std::string_view prefix = "[shared]";
        static constexpr std::string_view tid    = skr::rttr::type_name<T>();
        return push_sobjectptr(L, value, constexpr_join_v<prefix, tid>);
    }
    static skr::SObjectPtr<T> check(lua_State* L, int index)
    {
        return reinterpret_pointer_cast<T>(check_sobjectptr(L, index, ::skr::rttr::type_id<T>()));
    }
};

template <class T>
struct DefaultBindTrait<T, std::enable_if_t<std::is_enum_v<T>>> {
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

template <class T>
struct DefaultBindTrait<const T&, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::rttr::RTTRTraits<T>>>> {
    static int push(lua_State* L, const T& value)
    {
        return DefaultBindTrait<T>::push(L, value);
    }
    static const T& check(lua_State* L, int index)
    {
        return DefaultBindTrait<T>::check(L, index);
    }
};

template <class T>
int push(lua_State* L, const T& value)
{
    using TT = std::remove_const_t<std::remove_reference_t<T>>;
    if constexpr (skr::is_complete_v<BindTrait<T>>)
        return BindTrait<TT>::push(L, value);
    else
        return DefaultBindTrait<TT>::push(L, value);
}

template <class T>
decltype(auto) check(lua_State* L, int index, int& used)
{
    using TT = std::remove_const_t<std::remove_reference_t<T>>;
    if constexpr (skr::is_complete_v<BindTrait<T>>)
        return BindTrait<TT>::check(L, index, used);
    else
    {
        used = 1;
        return DefaultBindTrait<TT>::check(L, index);
    }
}

struct shared_userdata_t {
    void*      data;
    SPtr<void> shared;
};

template <>
struct BindTrait<float> {
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

template <>
struct BindTrait<double> {
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

#define BindInt(type)                                                 \
    template <>                                                       \
    struct BindTrait<type> {                                          \
        static int push(lua_State* L, type value)                     \
        {                                                             \
            lua_pushinteger(L, (int)value);                           \
            return 1;                                                 \
        }                                                             \
        static type check(lua_State* L, int index, int& used)         \
        {                                                             \
            used = 1;                                                 \
            return (type)luaL_checkinteger(L, index);                 \
        }                                                             \
        static type opt(lua_State* L, int index, type def, int& used) \
        {                                                             \
            used = 1;                                                 \
            return (type)luaL_optinteger(L, index, (int)def);         \
        }                                                             \
    };
BindInt(uint32_t);
BindInt(int32_t);
BindInt(uint64_t);
BindInt(int64_t);
#undef BindInt

template <>
struct BindTrait<bool> {
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

template <>
struct BindTrait<skr_guid_t> {
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

template <>
struct BindTrait<skr_resource_handle_t> {
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

template <class T>
struct BindTrait<resource::TResourceHandle<T>> {
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

template <>
struct BindTrait<skr::String> {
    static int push(lua_State* L, const skr::String& str)
    {
        return push_string(L, str);
    }

    static skr::String check(lua_State* L, int index, int& used)
    {
        used = 1;
        return check_string(L, index);
    }

    static skr::String opt(lua_State* L, int index, const skr::String& def)
    {
        return opt_string(L, index, def);
    }
};

template <>
struct BindTrait<skr::StringView> {
    static int push(lua_State* L, skr::StringView str)
    {
        return push_string(L, str);
    }
};

template <class T>
struct RefParam;

template <class T>
struct RefParam<T&> {
    using type = T;
    template <class Y>
    static Y& ref(Y& value)
    {
        return value;
    }
};

template <class T>
struct RefParam<T*> {
    using type = T;
    template <class Y>
    static Y* ref(Y& value)
    {
        return &value;
    }
};

template <class T>
using refed_t = typename RefParam<T>::type;

template <class T, class Y>
decltype(auto) ref(Y& value)
{
    return RefParam<T>::ref(value);
}

template <class T>
decltype(auto) deref(T value)
{
    if constexpr (std::is_reference_v<std::remove_const_t<T>>)
        return value;
    else
        return *value;
}

template <class T>
struct SharedUserdata {
    T* data;
    using shared_t = std::conditional_t<is_object_v<T>, SObjectPtr<T>, SPtr<T>>;
    shared_t shared;
    SharedUserdata(shared_t shared)
        : data(shared.get())
        , shared(std::move(shared))
    {
    }
};

template <class... Ts>
void Call(lua_State* L, Ts&&... args)
{
    auto oldTop = lua_gettop(L);
    int  n[]    = { BindTrait<Ts>::Push(L, std::forward<Ts>(args))... };
    int  nargs  = 0;
    for (int i = 0; i < sizeof...(Ts); ++i)
        nargs += n[i];
    if (lua_pcall(L, nargs, 0, 0) != LUA_OK)
    {
        lua_getglobal(L, "skr");
        lua_getfield(L, -1, "log_error");
        lua_pushvalue(L, -3);
        lua_call(L, 1, 0);
        lua_pop(L, 2);
    }
    lua_settop(L, oldTop);
}

template <class R, class... Ts>
R CallR(lua_State* L, Ts&&... args)
{
    auto oldTop = lua_gettop(L);
    int  n[]    = { BindTrait<Ts>::Push(L, std::forward<Ts>(args))... };
    int  nargs  = 0;
    for (int i = 0; i < sizeof...(Ts); ++i)
        nargs += n[i];
    if (lua_pcall(L, nargs, LUA_MULTRET, 0) != LUA_OK)
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
} // namespace skr::lua