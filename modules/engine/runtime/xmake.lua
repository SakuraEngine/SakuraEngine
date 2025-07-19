if (is_os("macosx") or is_os("linux")) then
    add_requires("libsdl 2.28.5", {configs = {shared = true}})
end
-- add_requires("cpu_features v0.9.0")

target("SkrRTMeta")
    set_group("00.utilities")
    set_kind("headeronly")
    codegen_generator({
        scripts = {
            -- ecs
            { file = "meta/ecs.ts" },
        }, 
        dep_files = {
            "meta/**.ts",
            "meta/**.py",
            "meta/**.mako"
        }
    })

shared_module("SkrRT", "SKR_RUNTIME")
    -- dependencies
    public_dependency("SkrTask")
    public_dependency("SkrGraphics")

    -- meta functional
    add_deps("SkrRTMeta")

    -- link system libs/frameworks
    add_linkdirs("$(builddir)/$(os)/$(arch)/$(mode)", {public = true})
    
    -- add source files
    add_includedirs("include", {public = true})
    add_defines("SUGOI_RESOURCE_SUPPORT", {public = false})
    -- add_files("src/**/build.*.c")
    add_files("src/**/build.*.cpp")
    if (is_os("macosx")) then 
        add_files("src/**/build.*.mm")
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "IOKit", {public = true})
    end

    skr_dbg_natvis_files("dbg/**.natvis")