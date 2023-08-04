set_xmakever("2.8.1")
set_project("SakuraEngine")

set_policy("build.ccache", false)
add_rules("plugin.compile_commands.autoupdate", { outputdir = ".vscode" }) -- xmake 2.7.4 

add_moduledirs("xmake/modules")
add_plugindirs("xmake/plugins")
add_repositories("skr-xrepo xrepo", {rootdir = os.scriptdir()})

set_languages(get_config("cxx_version"), get_config("c_version"))
add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")

includes("xmake/options.lua")
--includes("xmake/toolchains/prospero.lua")

engine_version = "0.1.0"
default_unity_batch_size = 16

includes("xmake/compile_flags.lua")
includes("xmake/rules.lua")

if (is_os("windows")) then 
    add_defines("UNICODE", "NOMINMAX", "_WINDOWS")
    add_defines("_GAMING_DESKTOP")
    add_defines("_CRT_SECURE_NO_WARNINGS")
    if (is_mode("release")) then
        set_runtimes("MD")
    elseif (is_mode("asan")) then
        add_defines("_DISABLE_VECTOR_ANNOTATION")
    else
        set_runtimes("MDd")
    end
elseif (is_os("macosx") or is_os("linux")) then
    add_requires("libsdl")
else
    -- ...
end

target("SkrRoot")
    set_kind("headeronly")
    -- core deps
    add_deps("SkrCompileFlags", {public = true})
    add_deps("boost", "tracyclient", {public = true})
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