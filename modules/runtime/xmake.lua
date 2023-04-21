if(has_config("shipping_one_archive")) then
    add_requires("eastl =3.20.2-skr", { configs = { runtime_shared = false } })
else
    add_requires("eastl =3.20.2-skr", { configs = { runtime_shared = true } })
end

add_requires("fmt =9.1.0-skr")
add_requires("lua =5.4.4-skr")
add_requires("simdjson =3.0.0-skr")

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

shared_module("SkrRT", "RUNTIME", engine_version)
    set_group("01.modules")
    add_deps("SkrRoot", {public = true})
    -- internal packages
    add_packages("fmt", "lua", "simdjson", {public = true, inherit = true})
    -- defs & flags
    add_defines(defs_list, {public = true})
    add_ldflags(project_ldflags, {public = true, force = true})
    add_cxflags(project_cxflags, {public = true, force = true})
    add_includedirs(include_dir_list, {public = true})
    add_includedirs(private_include_dir_list, {public = false})
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

    -- boost ctx
    do
        local platform = "x86_64"
        local os = "ms"
        local abi = "pe"
        local asm = "masm.asm"

        if is_arch("x86_64", "amd64", "x64") then 
            if is_config("pointer-size", "8") then 
                platform = "x86_64"
            else 
                platform = "i386"
            end
        end
        if is_arch("i386") then 
            platform = "i386"
        end
        if is_arch("arm.*") then 
            if is_config("pointer-size", "4") then 
                platform = "arm64"
            else
                platform = "arm"
            end
            os = "aapcs"
        end
        if is_arch("aarch64.*") then 
            platform = "arm64"
            os = "aapcs"
        end
        if is_os("windows") then
            if os == nil then
                os = "ms"
            end
            abi = "pe"
            if has_config("is_msvc") then 
                asm = "masm.asm"
            else 
                asm = "gas.S"
            end
        elseif is_os("linux") or is_plat("prospero") then 
            if os == nil then
                os = "sysv"
            end 
            abi = "elf"
            asm = "gas.S"
        elseif is_os("macosx") then
            os = "sysv"
            abi = "macho"
            asm = "gas.S"
        else 
            error("Unsupported os")
        end
        local canary = ""
        if is_config("ftl_fiber_canary_bytes") then 
            canary = "canary_"
        else 
            canary = ""
        end
        local file = canary..platform.."_"..os.."_"..abi.."_"..asm
        if asm == "masm.asm" and platform == "i386" then
            add_asflags("/safeseh")
        end
        add_files("$(projectdir)/thirdparty/boost_context/asm/make_"..file)
        add_files("$(projectdir)/thirdparty/boost_context/asm/jump_"..file)
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