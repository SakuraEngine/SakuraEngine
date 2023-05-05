set_xmakever("2.7.6")
add_repositories("skr-xrepo xrepo", {rootdir = os.scriptdir()})

set_project("SakuraEngine")

set_policy("build.ccache", false)

add_rules("plugin.compile_commands.autoupdate", { outputdir = ".vscode" }) -- xmake 2.7.4 

add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")
add_moduledirs("xmake/modules")

includes("xmake/options.lua")
--includes("xmake/toolchains/prospero.lua")

set_languages(get_config("cxx_version"), get_config("c_version"))

engine_version = "0.1.0"
default_unity_batch_size = 8

include_dir_list = {"include"}
private_include_dir_list = {}
source_list = {}
packages_list = {}
defs_list = {}
links_list = {}
generator_list = {}

includes("xmake/options_detect.lua")
includes("xmake/rules.lua")

if (is_os("windows")) then 
    add_defines("_GAMING_DESKTOP")
    add_defines("_WINDOWS")
    add_defines("UNICODE")
    add_defines("_UNICODE")
    add_defines("NOMINMAX")
    add_defines("_CRT_SECURE_NO_WARNINGS")
    if (is_mode("release")) then
        set_runtimes("MD")
    elseif (is_mode("asan")) then
        table.insert(defs_list, "_DISABLE_VECTOR_ANNOTATION")
    else
        set_runtimes("MDd")
    end
elseif (is_os("macosx") or is_os("linux")) then
    add_requires("python")
    add_requires("libsdl")
    add_requires("gtest")
else

end

target("SkrRoot")
    set_kind("headeronly")
    -- install sdks for windows platform
    libs_to_install = { "tracyclient" }
    if(os.host() == "windows") then
        table.insert(libs_to_install, "dstorage")
        table.insert(libs_to_install, "amdags")
        table.insert(libs_to_install, "nvapi")
        table.insert(libs_to_install, "nsight")
        table.insert(libs_to_install, "WinPixEventRuntime")
        table.insert(libs_to_install, "SDL2")
    end
    add_rules("utils.install-libs", { libnames = libs_to_install })
    -- core deps
    add_deps("boost", {public = true})
    -- generate codegen fences
    after_load(function(target)
        import("meta_codegen")
        if(has_config("use_async_codegen")) then
            meta_codegen.generate_fences(nil)
        end
    end)
    -- dispatch codegen task
    before_build(function(target)
        import("core.base.option")
        local targetname = option.get("target")
        local function upzip_tasks(targetname)
            import("core.project.task")
            task.run("run-codegen-jobs", {}, targetname)
        end
        if(has_config("use_async_codegen")) then
            import("core.base.scheduler")
            scheduler.co_start(upzip_tasks, targetname)
        else
            upzip_tasks(targetname)
        end
    end)
target_end()

includes("xmake/thirdparty.lua")
includes("tools/codegen/xmake.lua")

set_warnings("all")

includes("modules/xmake.lua")

if(has_config("build_samples")) then
    includes("samples/xmake.lua")
end
if(has_config("build_tools")) then
    includes("tools/xmake.lua")
end
if(has_config("build_editors")) then
    includes("editors/xmake.lua")
end
if(has_config("build_tests")) then
    includes("tests/xmake.lua")
end