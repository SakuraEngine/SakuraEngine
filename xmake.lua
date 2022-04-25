set_project("SakuraRuntime")

add_rules("mode.debug", "mode.release")
set_languages("c11", "cxx17")

add_requires("vulkan")

includes("xmake/options_detect.lua")

include_dir_list = {"include"}
source_list = {}
packages_list = {"vulkan"}
deps_list = {}
includes("xmake/mimalloc.lua")
includes("xmake/boost.lua")
includes("xmake/gsl.lua")
includes("xmake/EASTL.lua")
includes("xmake/DirectXMath.lua")
includes("xmake/SDL2.lua")
includes("xmake/imgui.lua")
includes("xmake/tracy.lua")
includes("xmake/wasm3.lua")

set_warnings("all")

target("SkrRT")
    add_rules("c++")
    set_kind("static")
    add_deps(deps_list)
    add_packages(packages_list, {public = true})
    add_includedirs(include_dir_list, {public = true})
    add_files(source_list)
    add_files("src/**/build.*.c", "src/**/build.*.cpp")
    add_cxflags(project_cxflags)
    if (is_os("macosx")) then 
        add_defines("SAKURA_TARGET_PLATFORM_MACOS")
        add_mxflags(project_mxflags)
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "Metal", "IOKit")
        add_files("src/**/build.*.m", "src/**/build.*.mm")
    end
    before_build(function(target)
        import("core.project.task")
        task.run("unzip-tracyclient")
        task.run("unzip-wasm3")
    end)
    add_links("TracyClient")
    add_links("m3", "uv_a", "uvwasi_a")

includes("samples/xmake.lua")