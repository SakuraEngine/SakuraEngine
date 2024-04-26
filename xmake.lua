set_xmakever("2.8.1")
set_project("SakuraEngine")

engine_version = "0.1.0"
default_unity_batch = 16

add_rules("plugin.compile_commands.autoupdate", { outputdir = ".vscode" }) -- xmake 2.7.4 

-- setup warnings
set_warnings("all", "extra")
add_cxxflags("-Wdeprecated-declarations", {tools={"gcc", "clang", "clang_cl"}})
add_cxflags("-Wno-unused-parameter", {tools={"gcc", "clang", "clang_cl"}})
add_cxflags("-Wno-sign-compare", {tools={"gcc", "clang", "clang_cl"}})
add_cxflags("-Wno-ignored-qualifiers", {tools={"gcc", "clang", "clang_cl"}})
add_cxflags("-Wno-deprecated-copy-with-user-provided-copy", {tools={"gcc", "clang", "clang_cl"}})
add_cflags("-Wno-unused-variable", {tools={"gcc", "clang", "clang_cl"}})
add_cxflags("cl::/wd4100") -- Wno-unused-parameter
add_cxflags("cl::/wd4018") -- Wno-sign-compare
add_cxflags("cl::/wd4389") -- Wno-sign-compare ==
add_cxflags("cl::/wd4245") -- = lost data for signed -> unsigned
add_cxflags("cl::/wd4244") -- = lost data for bigger to smaller
add_cxflags("cl::/wd4267") -- = lost data for bigger to smaller
add_cxflags("cl::/wd4127") -- conditional expression is constant
add_cxflags("cl::/wd4706") -- assignment within conditional expression
add_cxflags("cl::/wd4458") -- declaration of 'xxxx' hides class member
add_cxflags("cl::/wd4275") -- non dll-interface struct 'xxxx' used as base for dll-interface struct 'xxxx'
add_cxflags("cl::/wd4201") -- nonstandard extension used: nameless struct/union
add_cxflags("cl::/wd4624") -- destructor was implicitly defined as deleted
add_cxflags("cl::/wd4819") -- The file contains a character that cannot be represented in the current code page (936). Save the file in Unicode format to prevent data loss
add_cxflags("cl::/wd4456") -- declaration of 'xxxx' hides previous local declaration
add_cxflags("cl::/wd4457") -- declaration of 'xxxx' hides function parameter
add_cxflags("cl::/wd4459") -- declaration of 'xxxx' hides global declaration
add_cxflags("cl::/wd4324") -- structure was padded due to alignment specifier
add_cxflags("cl::/wd4702") -- unreachable code
add_cflags("cl::/wd4189") -- local variable is initialized but not referenced

set_policy("build.ccache", false)
set_policy("check.auto_ignore_flags", false)
set_policy("build.warning", true)

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