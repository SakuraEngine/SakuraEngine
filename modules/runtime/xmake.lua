if(has_config("shipping_one_archive")) then
    add_requires("eastl >=3.20.2-skr", { configs = { runtime_shared = false } })
else
    add_requires("eastl >=3.20.2-skr", { configs = { runtime_shared = true } })
end

add_requires("parallel-hashmap >=1.3.4-skr")
add_requires("boost-context >=0.1.0-skr")
add_requires("fmt >=9.1.0-skr")
add_requires("lua >=5.4.4-skr")
add_requires("simdjson >=3.0.0-skr")

target("SkrDependencyGraph")
    set_group("01.modules")
    add_deps("SkrRoot", {public = true})
    add_packages("eastl", {public = true, inherit = true})
    set_exceptions("no-cxx")
    add_rules("skr.static_module", {api = "SKR_DEPENDENCY_GRAPH"})
    set_optimize("fastest")
    add_files("dependency_graph/**/dependency_graph.cpp")
    add_defines(defs_list, {public = true})
    add_includedirs("dependency_graph", {public = true})
    add_includedirs(include_dir_list, {public = true})
    add_includedirs(private_include_dir_list, {public = false})

target("SkrRTStatic")
    set_group("01.modules")
    set_optimize("fastest")
    set_exceptions("no-cxx")
    add_deps("SkrRoot", {public = true})
    add_defines("RUNTIME_API=RUNTIME_IMPORT", "RUNTIME_LOCAL=error")
    add_packages("eastl", {public = true, inherit = true})
    add_packages("parallel-hashmap", "fmt", "simdjson", {public = true, inherit = true})
    add_rules("skr.static_module", {api = "SKR_RUNTIME_STATIC"})
    add_defines(defs_list, {public = true})
    add_includedirs(include_dir_list, {public = true})
    add_includedirs(private_include_dir_list, {public = false})
    add_files("src_static/**/build.*.cpp")
    -- add_files("src_static/**/build.*.c")

shared_module("SkrRT", "RUNTIME", engine_version)
    set_group("01.modules")
    add_deps("SkrRTStatic", {public = true, inherit = true})
    add_includedirs(private_include_dir_list, {public = false})

    -- internal packages
    add_packages("boost-context", "lua", {public = true, inherit = true})

    -- add source files
    add_files(source_list)
    add_files("src/**/build.*.c", "src/**/build.*.cpp")
    if (is_os("macosx")) then 
        add_files("src/**/build.*.m", "src/**/build.*.mm")
    end

    -- add deps & links
    add_deps("SkrDependencyGraph", {public = false})
    add_deps("vulkan", {public = true})
    add_packages(packages_list, {public = true})

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
    add_links(links_list, {public = true})
    if (is_os("windows")) then 
        add_syslinks("advapi32", "user32", "shell32", "Ole32", {public = true})
    end
    if (is_os("macosx")) then 
        add_mxflags(project_cxflags, project_mxflags, {public = true, force = true})
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "Metal", "IOKit", {public = true})
    end
    if has_config("is_unix") then 
        add_syslinks("pthread")
    end
    
    -- add FTL source 
    add_files("$(projectdir)/thirdparty/FiberTaskingLib/source/build.*.cpp")
    
    -- add marl source
    add_defines("MARL_USE_EASTL", {public = true})
    local marl_source_dir = "$(projectdir)/thirdparty/marl"
    add_files(marl_source_dir.."/src/build.*.cpp")
    if not has_config("is_msvc") then 
        add_files(marl_source_dir.."/src/**.c")
        add_files(marl_source_dir.."/src/**.S")
    end
    add_includedirs("$(projectdir)/thirdparty/marl/include", {public = true})
    add_includedirs("$(projectdir)/thirdparty/marl/include/marl", {public = true})

    -- install dxc on windows platform
    if (is_os("windows")) then 
        add_rules("utils.install-libs", { libnames = {"dxc"} })
        add_linkdirs("$(buildir)/$(os)/$(arch)/$(mode)", {public=true})
        add_links("nvapi_x64", {public = true})
        add_links("WinPixEventRuntime", {public = true})
        -- we do not support x86 windows
        -- add_links("$(buildir)/$(os)/$(arch)/$(mode)/nvapi_x86")
    end

    -- cpu info private include dir
    add_includedirs("include/platform/cpu", {public = false})