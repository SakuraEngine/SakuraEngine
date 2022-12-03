#include "lua/lua.hpp"
#include "containers/string.hpp"
#include "containers/sptr.hpp"

namespace skr::lua
{
    RUNTIME_API int push_guid(lua_State* L, const skr_guid_t* guid);
    RUNTIME_API const skr_guid_t* check_guid(lua_State* L, int index);
    RUNTIME_API int push_string(lua_State* L, const skr::string& str);
    RUNTIME_API int push_string(lua_State* L, skr::string_view str);
    RUNTIME_API skr::string check_string(lua_State* L, int index);
    RUNTIME_API int push_resource(lua_State* L, const skr_resource_handle_t* resource);
    RUNTIME_API const skr_resource_handle_t* check_resource(lua_State* L, int index);
    // TODO: how should we handle math operations in lua?
    // RUNTIME_API int push_float2(lua_State* L, const skr_float2_t* float2);
    // RUNTIME_API skr_float2_t check_float2(lua_State* L, int index);
    // RUNTIME_API int push_float3(lua_State* L, const skr_float3_t* float3);
    // RUNTIME_API skr_float3_t check_float3(lua_State* L, int index);
    // RUNTIME_API int push_float4(lua_State* L, const skr_float4_t* float4);
    // RUNTIME_API skr_float4_t check_float4(lua_State* L, int index);

    template<class T>
    struct BindTrait;

    template<class T>
    int push(lua_State* L, const T& value)
    {
        return BindTrait<T>::push(L, &value);
    }

    template<class T>
    T check(lua_State* L, int index)
    {
        return BindTrait<T>::check(L, index);
    }

    template<class T>
    void bind(lua_State* L)
    {
        BindTrait<T>::bind(L);
    }

    struct shared_userdata_t
    {
        void* data;
        SPtr<void> shared;
    };

    template<class T>
    struct BindTrait<skr::SPtr<T>>
    {
        static int push(lua_State* L, skr::SPtr<T> value)
        {
            return BindTrait<T>::push_shared(L);
        }

        static skr::SPtr<T> check(lua_State* L, int index)
        {
            return BindTrait<T>::check_shared(L);
        }

        static void bind(lua_State* L)
        {
            BindTrait<T>::bind_shared(L);
        }
    };

    
    template<class T>
    struct BindTrait<skr::SObjectPtr<T>>
    {
        static int push(lua_State* L, skr::SPtr<T> value)
        {
            return BindTrait<T>::push_shared(L);
        }

        static skr::SPtr<T> check(lua_State* L, int index)
        {
            return BindTrait<T>::check_shared(L);
        }

        static void bind(lua_State* L)
        {
            BindTrait<T>::bind_shared(L);
        }
    };

    template<>
    struct BindTrait<skr_guid_t>
    {
        static int push(lua_State* L, const skr_guid_t* guid)
        {
            return push_guid(L, guid);
        }

        static const skr_guid_t* check(lua_State* L, int index)
        {
            return check_guid(L, index);
        }
    };

    template<>
    struct BindTrait<skr_resource_handle_t>
    {
        static int push(lua_State* L, const skr_resource_handle_t* resource)
        {
            return push_resource(L, resource);
        }

        static const skr_resource_handle_t* check(lua_State* L, int index)
        {
            return check_resource(L, index);
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
    };

    template<>
    struct BindTrait<skr::string_view>
    {
        static int push(lua_State* L, skr::string_view str)
        {
            return push_string(L, str);
        }
    };
}