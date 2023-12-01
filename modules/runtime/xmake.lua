if(has_config("shipping_one_archive")) then
    add_requires("eastl >=2023.5.18-skr", { configs = { runtime_shared = false } })
else
    add_requires("eastl >=2023.5.18-skr", { configs = { runtime_shared = true } })
end

add_requires("lemon 1.3.1")
add_requires("parallel-hashmap >=1.3.11-skr")
add_requires("boost-context >=0.1.0-skr")
add_requires("simdjson >=3.0.0-skr")
add_requires("luau", { configs = { extern_c = true }})

target("SkrRTStatic")
    set_group("01.modules")
    set_optimize("fastest")
    set_exceptions("no-cxx")
    add_deps("SkrRoot", "SkrBase", "SkrMemory", {public = true})
    add_defines("SKR_RUNTIME_API=SKR_IMPORT", "SKR_RUNTIME_LOCAL=error")
    add_packages("eastl", "parallel-hashmap", "simdjson", {public = true, inherit = true})
    add_packages("lemon", {public = false, inherit = false})
    add_rules("skr.static_module", {api = "SKR_RUNTIME_STATIC"})
    add_includedirs("include", {public = true})
    set_pcxxheader("src_static/pch.hpp")
    add_files("src_static/**/build.*.cpp")
    -- add_files("src_static/**/build.*.c")

shared_module("SkrRT", "SKR_RUNTIME", engine_version)
    set_group("01.modules")
    add_deps("SkrRTStatic", "SkrMemory", {public = true, inherit = true})
    add_defines("SKR_RUNTIME_API=SKR_EXPORT", "SKR_RUNTIME_LOCAL=error")

    -- internal packages
    add_packages("boost-context", "luau", {public = true, inherit = true})

    -- add source files
    add_files("src/**/build.*.c", "src/**/build.*.cpp")
    if (is_os("macosx")) then 
        add_files("src/**/build.*.m", "src/**/build.*.mm")
    end

    -- https://github.com/xmake-io/xmake/issues/3912
    -- need xmake v2.8.1 to enable pch with objcpp
    local mxx_pch = xmake.version():ge("2.8.1")
    if (not is_os("macosx") or mxx_pch) then 
        set_pcxxheader("src/pch.hpp")
    end

    -- add deps & links
    add_deps("vulkan", {public = true})

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
    if (is_os("macosx")) then 
        -- add_mxflags(project_mxflags, {public = true, force = true})
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "Metal", "IOKit", {public = true})
    end

    -- add FTL source 
    add_files("$(projectdir)/thirdparty/FiberTaskingLib/source/build.*.cpp")
    
    -- add marl source
    -- add_defines("MARL_USE_EASTL", {public = true})
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
        table.insert(libs_to_install, "dstorage-1.2.1")
        table.insert(libs_to_install, "gns")
        table.insert(libs_to_install, "amdags")
        table.insert(libs_to_install, "nvapi")
        table.insert(libs_to_install, "nsight")
        table.insert(libs_to_install, "WinPixEventRuntime")
        table.insert(libs_to_install, "SDL2")
        table.insert(libs_to_install, "dxc")
    end
    add_rules("utils.install-libs", { libnames = libs_to_install })

    if (is_os("windows")) then 
        add_linkdirs("$(buildir)/$(os)/$(arch)/$(mode)", {public=true})
        add_links("nvapi_x64", {public = true})
        add_links("WinPixEventRuntime", {public = true})
    end

    -- cpu info private include dir
    add_includedirs("include/SkrRT/cpuinfo", {public = false})

    -- mimalloc private include dir
    add_includedirs("src", {public = false})