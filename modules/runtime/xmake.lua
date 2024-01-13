add_requires("boost-context >=0.1.0-skr")
-- add_requires("cpu_features v0.9.0")

target("SkrRTStatic")
    set_group("01.modules")
    set_optimize("fastest")
    set_exceptions("no-cxx")
    add_deps("SkrRoot", "SkrCore", {public = true})
    if(not has_config("shipping_one_archive")) then
        add_defines("SKR_RUNTIME_API=SKR_IMPORT", "SKR_RUNTIME_LOCAL=error")
    else
        add_defines("SKR_RUNTIME_API=", "SKR_RUNTIME_LOCAL=error")
    end
    add_rules("skr.static_module", {api = "SKR_RUNTIME_STATIC"})
    add_includedirs("include", {public = true})
    set_pcxxheader("src_static/pch.hpp")
    add_files("src_static/**/build.*.cpp")
    -- add_files("src_static/**/build.*.c")

shared_module("SkrRT", "SKR_RUNTIME", engine_version)
    set_group("01.modules")
    public_dependency("SkrCore", engine_version)
    public_dependency("SkrGraphics", engine_version)
    add_deps("SkrRTStatic", {public = true, inherit = true})
    add_defines("SKR_RUNTIME_API=SKR_EXPORT", "SKR_RUNTIME_LOCAL=error")

    -- internal packages
    add_packages("boost-context", {public = true, inherit = true})

    -- add source files
    set_pcxxheader("src/pch.hpp")
    add_files("src/**/build.*.c", "src/**/build.*.cpp")
    if (is_os("macosx")) then 
        add_files("src/**/build.*.mm")
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "IOKit", {public = true})
    end

    -- runtime compile definitions
    after_load(function (target,  opt)
        if (target:get("kind") == "shared") then
            import("core.project.project")
            target:add("defines", "MARL_DLL", {public = true})
            target:add("defines", "MARL_BUILDING_DLL")
        end
    end)

    -- link system libs/frameworks
    add_linkdirs("$(buildir)/$(os)/$(arch)/$(mode)", {public = true})
    if (is_os("windows")) then 
        add_syslinks("advapi32", "user32", "shell32", "Ole32", "Shlwapi", {public = true})
    else
        add_syslinks("pthread")
    end

    -- add FTL source 
    add_files("$(projectdir)/thirdparty/FiberTaskingLib/source/build.*.cpp")
    
    -- add marl source
    local marl_source_dir = "$(projectdir)/thirdparty/marl"
    add_files(marl_source_dir.."/src/build.*.cpp")
    if not is_os("windows") then 
        add_files(marl_source_dir.."/src/**.c")
        add_files(marl_source_dir.."/src/**.S")
    end
    add_includedirs("$(projectdir)/thirdparty/marl/include", {public = true})

    -- install sdks for windows platform
    libs_to_install = {}
    if(os.host() == "windows") then
        table.insert(libs_to_install, "gns")
        table.insert(libs_to_install, "SDL2")
    end
    add_rules("utils.install-libs", { libnames = libs_to_install })