#include "lua/lua.hpp"
#include "containers/string.hpp"


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
}