if (is_os("macosx") or is_os("linux")) then
    add_requires("libsdl 2.28.5", {configs = {shared = true}})
end
-- add_requires("cpu_features v0.9.0")

target("SkrRTMeta")
    set_kind("headeronly")
    codegen_generator({
        scripts = {
            -- ecs
            { file = "meta/ecs/ecs.py" },
        }, 
        dep_files = {
            "meta/**.py",
            "meta/**.mako"
        }
    })

shared_module("SkrRT", "SKR_RUNTIME", engine_version)
    -- dependencies
    public_dependency("SkrTask", engine_version)
    public_dependency("SkrGraphics", engine_version)

    -- meta functional
    add_deps("SkrRTMeta")

    -- link system libs/frameworks
    add_linkdirs("$(buildir)/$(os)/$(arch)/$(mode)", {public = true})
    
    -- add source files
    add_includedirs("include", {public = true})
    -- add_files("src/**/build.*.c")
    add_files("src/**/build.*.cpp")
    if (is_os("macosx")) then 
        add_files("src/**/build.*.mm")
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "IOKit", {public = true})
    end

    -- install sdks for windows platform
    libs_to_install = {}
    if(os.host() == "windows") then
        table.insert(libs_to_install, "gns")
    end
    add_rules("utils.install_libraries", { libnames = libs_to_install })