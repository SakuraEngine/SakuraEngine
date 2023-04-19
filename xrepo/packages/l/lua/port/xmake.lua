local lua_include_dir = "lua/include"
local lua_include_dir_private = "lua/include/lua"
local lua_source_dir = "lua/src"

target("lua")
    set_kind("static")
    set_optimize("fastest")
    add_headerfiles(lua_include_dir.."/(**.h)")
    add_headerfiles(lua_include_dir.."/(**.hpp)")
    add_includedirs(lua_include_dir, {public = true})
    add_includedirs(lua_include_dir_private, {public = false})
    add_files(lua_source_dir.."/*.c")