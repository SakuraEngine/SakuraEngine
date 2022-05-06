set_project("Sakura.Runtime")

add_rules("mode.debug", "mode.release")
add_moduledirs("xmake/modules")

option("build_samples")
    set_default(true)
    set_showmenu(true)
    set_description("Toggle to build samples of SakuraRuntime")
option_end()

option("build_tests")
    set_default(true)
    set_showmenu(true)
    set_description("Toggle to build tests of SakuraRuntime")
option_end()

set_languages("c11", "cxx17")

include_dir_list = {"include"}
source_list = {}
packages_list = {}
deps_list = {}
links_list = {}

includes("xmake/options_detect.lua")
includes("xmake/rules.lua")
includes("xmake/thirdparty.lua")

set_warnings("all")
if (is_os("windows")) then 
    add_defines("UNICODE")
    add_defines("NOMINMAX")
    add_defines("_CRT_SECURE_NO_WARNINGS")
    if (is_mode("release")) then
        set_runtimes("MD")
    else
        set_runtimes("MDd")
    end
end

target("SkrRT") 
    set_kind("static")
    add_rules("c++.reflection", {
        files = {"include/resource/**.h", "include/resource/**.hpp"},
        rootdir = "include/"
    })
    add_deps(deps_list)
    add_packages(packages_list, {public = true})
    add_includedirs(include_dir_list, {public = true})
    add_files(source_list)
    add_files("src/**/build.*.c", "src/**/build.*.cpp")
    add_cxflags(project_cxflags, {public = true})
    -- fetch vk includes
    add_rules("utils.fetch-vk-includes")
    -- add internal shaders
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders", 
        dxil_outdir = "/../resources/shaders"})
    add_files("src/**/*.hlsl")
    -- link system libs/frameworks
    if (is_os("windows")) then 
        add_links("advapi32", "Shcore")
    end
    if (is_os("macosx")) then 
        add_mxflags(project_cxflags, project_mxflags, {public = true})
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "Metal", "IOKit")
        add_files("src/**/build.*.m", "src/**/build.*.mm")
    end
    -- unzip & link sdks
    before_build(function(target)
        import("core.project.task")
        task.run("unzip-tracyclient")
        task.run("unzip-wasm3")
        task.run("unzip-gfx-sdk")
    end)
    add_links(links_list)
    add_links("TracyClient")
    add_links("m3", "uv_a", "uvwasi_a")
target_end()

if(has_config("build_tools")) then
    includes("tools/xmake.lua")
end
if(has_config("build_samples")) then
    includes("samples/xmake.lua")
end
if(has_config("build_tests")) then
    includes("tests/xmake.lua")
end