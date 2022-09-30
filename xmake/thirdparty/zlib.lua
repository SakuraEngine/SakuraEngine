zlib_include_dir = "$(projectdir)/thirdparty/zlib/include"
zlib_lib_dir = "$(projectdir)/thirdparty/zlib/lib"

target("zlib")
    set_group("00.thirdparty")
    set_kind("headeronly")
    add_includedirs(zlib_include_dir, {public=true})
    if (is_os("windows")) then 
        add_linkdirs(zlib_lib_dir.."/windows/x64", {public=true})
        add_links("zlibstatic", {public=true})
    end
    if (is_os("macosx")) then 
        add_linkdirs(zlib_lib_dir.."/macos/x86_64", {public=true})
        add_links("z", {public=true})
    end