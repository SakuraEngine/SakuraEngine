lua_include_dir = "$(projectdir)/thirdparty/lua/include"
lua_include_dir_private = "$(projectdir)/thirdparty/lua/include/lua"
lua_source_dir = "$(projectdir)/thirdparty/lua/src"

target("lua")
    set_group("00.thirdparty")
    set_kind("static")
    --set_optimize("fastest")
    add_files(lua_source_dir.."/*.c")
    add_includedirs(lua_include_dir, {public = true})
    add_includedirs(lua_include_dir_private, {public = false})