add_requires("boost-context >=0.1.0-skr")
-- add_requires("cpu_features v0.9.0")

static_component("SkrSerde", "SkrRT")
    set_optimize("fastest")
    -- set_pcxxheader("serde/pch.hpp")
    add_files("serde/build.*.cpp")
    public_dependency("SkrCore", engine_version)
    
shared_module("SkrRT", "SKR_RUNTIME", engine_version)
    -- dependencies
    public_dependency("SkrTask", engine_version)
    public_dependency("SkrGraphics", engine_version)

    -- link system libs/frameworks
    add_linkdirs("$(buildir)/$(os)/$(arch)/$(mode)", {public = true})
    if (is_os("windows")) then 
        add_syslinks("advapi32", "user32", "shell32", "Ole32", "Shlwapi", {public = true})
    else
        add_syslinks("pthread")
    end
    
    -- add source files
    add_includedirs("include", {public = true})
    set_pcxxheader("src/pch.hpp")
    add_files("src/**/build.*.c", "src/**/build.*.cpp")
    if (is_os("macosx")) then 
        add_files("src/**/build.*.mm")
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "IOKit", {public = true})
    end

    -- install sdks for windows platform
    libs_to_install = {}
    if(os.host() == "windows") then
        table.insert(libs_to_install, "gns")
        table.insert(libs_to_install, "SDL2")
    end
    add_rules("utils.install-libs", { libnames = libs_to_install })