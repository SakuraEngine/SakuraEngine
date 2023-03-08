includes("ft-hb-icu/freetype.lua")
includes("ft-hb-icu/icu.lua")
includes("ft-hb-icu/harfbuzz.lua")

shared_module("SkrGui", "SKR_GUI", engine_version)
    set_group("01.modules")
    add_deps("freetype", "harfbuzz", "icu", {public=false})
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_includedirs("src", {public=false})
    add_files("src/**.cpp")
    if (is_plat("windows")) then
        add_cxflags("/wd4267", "/wd4244", "/wd4018","/source-charset:utf-8", {public=false})
    end