add_requires("freetype skr")
add_requires("icu skr")
add_requires("harfbuzz skr")

target("nanovg")
    set_group("00.thirdparty")
    set_kind("static")
    set_optimize("fastest")
    add_includedirs("nanovg", {public=true})
    add_files("nanovg/nanovg.cpp")

shared_module("SkrGui", "SKR_GUI", engine_version)
    set_group("01.modules")
    add_packages("freetype", "icu", "harfbuzz")
    add_deps("nanovg", {public=false})
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_includedirs("src", {public=false})
    add_files("src/**.cpp")
    if (is_plat("windows")) then
        add_cxflags("/wd4267", "/wd4244", "/wd4018","/source-charset:utf-8", {public=false})
    end