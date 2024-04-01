set_xmakever("2.8.1")
set_project("SakuraEngine")

engine_version = "0.1.0"
default_unity_batch = 16

add_rules("plugin.compile_commands.autoupdate", { outputdir = ".vscode" }) -- xmake 2.7.4 

set_warnings("all")
set_policy("build.ccache", false)
set_policy("check.auto_ignore_flags", false)

add_moduledirs("xmake/modules")
add_plugindirs("xmake/plugins")
add_repositories("skr-xrepo xrepo", {rootdir = os.scriptdir()})

set_languages(get_config("cxx_version"), get_config("c_version"))
add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")

includes("xmake/options.lua")

option("project_script")
    set_default("project.lua")
    set_showmenu(true)
option_end()

if get_config("project_script") and os.exists(get_config("project_script")) then
    includes(get_config("project_script"))
else
    includes("./xmake/project.default.lua")
end

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
end

includes("xmake/thirdparty.lua")
includes("modules/xmake.lua")

if build_part("samples") then
    includes("samples/xmake.lua")
end

if build_part("editors") then
    includes("editors/xmake.lua")
end

if build_part("tests") then
    includes("tests/xmake.lua")
end