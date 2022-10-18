set_project("Sakura.Runtime")

add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_moduledirs("xmake/modules")

includes("xmake/options.lua")

set_languages("c11", "cxx17")

default_unity_batch_size = 8

include_dir_list = {"include"}
source_list = {}
packages_list = {}
defs_list = {}
links_list = {}
generator_list = {}

includes("xmake/options_detect.lua")
includes("xmake/rules.lua")

target("SkrRoot")
    set_kind("headeronly")
    -- core deps
    add_deps("simdjson", "gsl", "fmt", "ghc_fs", "boost", "gsl", {public = true})
    -- unzip & link sdks
    before_build(function(target)
        import("core.base.option")
        local targetname = option.get("target")

        import("core.base.scheduler")
        local function upzip_tasks(targetname)
            import("core.project.task")

            task.run("run-codegen-jobs", {}, targetname)
            task.run("unzip-tracyclient")
            --task.run("unzip-wasm3")
            task.run("unzip-platform-sdks")
        end
        scheduler.co_start(upzip_tasks, targetname)
    end)
target_end()

includes("xmake/thirdparty.lua")
includes("tools/codegen/xmake.lua")

set_warnings("all")

if (is_os("windows")) then 
    add_defines("_WINDOWS")
    add_defines("UNICODE")
    add_defines("_UNICODE")
    add_defines("NOMINMAX")
    add_defines("_CRT_SECURE_NO_WARNINGS")
    if (is_mode("release")) then
        set_runtimes("MD")
    else
        set_runtimes("MDd")
    end
end

includes("modules/xmake.lua")

if(has_config("build_samples")) then
    includes("samples/xmake.lua")
end
if(has_config("build_tools")) then
    includes("tools/xmake.lua")
end
if(has_config("build_editors")) then
    includes("xmake/thirdparty-ed.lua")
    includes("editors/xmake.lua")
end
if(has_config("build_tests")) then
    includes("tests/xmake.lua")
end