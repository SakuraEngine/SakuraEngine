set_xmakever("2.9.2")
set_project("SakuraEngine")

engine_version = "0.1.0"
default_unity_batch = 16

-- will generate bad compile_commands.json
-- add_rules("plugin.compile_commands.autoupdate", { outputdir = ".vscode" }) -- xmake 2.7.4 

set_warnings("all")
set_policy("build.ccache", false)
set_policy("build.warning", true)
set_policy("check.auto_ignore_flags", false)

add_moduledirs("xmake/modules")
add_plugindirs("xmake/plugins")
add_repositories("skr-xrepo xrepo", {rootdir = os.scriptdir()})

set_languages(get_config("cxx_version"), get_config("c_version"))
add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")

includes("xmake/options.lua")
includes("xmake/compile_flags.lua")
includes("xmake/rules.lua")

-- add global rules to all targets
add_rules("DisableTargets")

if os.exists("project.lua") then
    includes("project.lua")
else
    includes("./xmake/project.default.lua")
end

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
end

includes("xmake/thirdparty.lua")
includes("modules/xmake.lua")
includes("samples/xmake.lua")
includes("editors/xmake.lua")
includes("tests/xmake.lua")