if (is_os("macosx") or is_os("linux")) then
    add_requires("libsdl 2.28.5", {configs = {shared = true}})
end
-- add_requires("cpu_features v0.9.0")
    
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
    add_files("src/**/build.*.c", "src/**/build.*.cpp")
    if (is_os("macosx")) then 
        add_files("src/**/build.*.mm")
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "IOKit", {public = true})
    end

    -- add SDL2
    if (is_os("windows")) then 
        add_links("SDL2", {public = true})
        sdl2_includes_dir = "$(projectdir)/thirdparty/SDL2"
        add_includedirs(sdl2_includes_dir, {public = true})
    elseif (is_os("macosx") or is_os("linux")) then
        add_packages("libsdl", {public = true})
    end

    -- install sdks for windows platform
    libs_to_install = {}
    if(os.host() == "windows") then
        table.insert(libs_to_install, "gns")
        table.insert(libs_to_install, "SDL2")
    end
    add_rules("utils.install-libs", { libnames = libs_to_install })

private_pch("SkrRT")
    add_files("src/pch.hpp")

shared_pch("SkrRT")
    add_files("include/SkrRT/**.hpp")
    add_files("include/SkrRT/**.h")