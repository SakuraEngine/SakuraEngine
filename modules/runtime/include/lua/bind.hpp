#include "lua/bind_fwd.hpp"
#include "containers/string.hpp"
#include "containers/sptr.hpp"
#include "resource/resource_handle.h"
#include "utils/traits.hpp"
#include "utils/join.hpp"

namespace skr::lua
{
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

    template<class T>
    struct BindTrait<T*, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::type::type_id<T>>>>
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
    struct BindTrait<T, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::type::type_id<T>>>>
    {
        static int push(lua_State* L, const T& value)
        {
            static constexpr std::string_view prefix = "[unique]";
            return push_unknown_value(L, value, join_v<prefix, skr::type::type_id<T>::str()>, sizeof(T), +[](void* dst, const void* src) {
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
    struct BindTrait<skr::SPtr<T>, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::type::type_id<T>>>>
    {
        static int push(lua_State* L, const skr::SPtr<T>& value)
        {
            static constexpr std::string_view prefix = "[shared]";
            return push_sptr(L, value, join_v<prefix, skr::type::type_id<T>::str()>);
        }
        static skr::SPtr<T> check(lua_State* L, int index)
        {
            return reinterpret_pointer_cast<T>(check_sptr(L, index, skr::type::type_id<T>::get()));
        }
    };

    template<class T>
    struct BindTrait<skr::SObjectPtr<T>, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::type::type_id<T>>>>
    {
        static int push(lua_State* L, const skr::SObjectPtr<T>& value)
        {
            static constexpr std::string_view prefix = "[shared]";
            return push_sobjectptr(L, value, join_v<prefix, skr::type::type_id<T>::str()>);
        }
        static skr::SObjectPtr<T> check(lua_State* L, int index)
        {
            return reinterpret_pointer_cast<T>(check_sobjectptr(L, index, skr::type::type_id<T>::get()));
        }
    };

    template <class T>
    struct BindTrait<T, std::enable_if_t<std::is_enum_v<T>>>
    {
        using type = T;
        static int push(lua_State* L, type value)
        {
            lua_pushinteger(L, static_cast<long long>(value));
            return 1;
        }
        static type check(lua_State* L, int index)
        {
            return static_cast<type>(luaL_checkinteger(L, index));
        }
        static type opt(lua_State* L, int index, type def)
        {
            return static_cast<type>(luaL_optinteger(L, index, static_cast<long long>(def)));
        }
    };

    
    template<class T>
    struct BindTrait<const T&, std::enable_if_t<!std::is_enum_v<T> && skr::is_complete_v<skr::type::type_id<T>>>>
    {
        static int push(lua_State* L, const T& value)
        {
            return BindTrait<T>::push(L, value);
        }
        static const T& check(lua_State* L, int index)
        {
            return BindTrait<T>::check(L, index);
        }
    };

    template<class T>
    int push(lua_State* L, const T& value)
    {
        return BindTrait<T>::push(L, value);
    }

    template<class T>
    T check(lua_State* L, int index)
    {
        return BindTrait<T>::check(L, index);
    }

    struct shared_userdata_t
    {
        void* data;
        SPtr<void> shared;
    };

    template<>
    struct BindTrait<skr_guid_t>
    {
        static int push(lua_State* L, const skr_guid_t& guid)
        {
            return push_guid(L, &guid);
        }

        static const skr_guid_t check(lua_State* L, int index)
        {
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

        static const skr_resource_handle_t check(lua_State* L, int index)
        {
            return *check_resource(L, index);
        }

        static const skr_resource_handle_t opt(lua_State* L, int index, const skr_resource_handle_t& def)
        {
            return *opt_resource(L, index, &def);
        }
    };

    template<>
    struct BindTrait<skr::string>
    {
        static int push(lua_State* L, const skr::string& str)
        {
            return push_string(L, str);
        }

        static skr::string check(lua_State* L, int index)
        {
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
}