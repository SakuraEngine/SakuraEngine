target("SkrDependencyGraph")
    set_group("01.modules")
    add_deps("SkrRoot", {public = true})
    set_exceptions("no-cxx")
    add_rules("skr.static_module", {api = "SKR_DEPENDENCY_GRAPH"})
    set_optimize("fastest")
    add_files("src/**/dependency_graph.cpp")
    add_files("src/**/boost_exception.cpp")
    add_defines(defs_list, {public = true})
    add_includedirs(include_dir_list, {public = true})

shared_module("SkrRT", "RUNTIME", engine_version)
    set_group("01.modules")
    add_deps("SkrRoot", {public = true})
    add_defines(defs_list, {public = true})
    add_cxflags(project_cxflags, {public = true, force = true})
    add_includedirs(include_dir_list, {public = true})
    add_files(source_list)
    add_files("src/**/build.*.c", "src/**/build.*.cpp")
    -- add deps & links
    add_deps("SkrDependencyGraph", {public = false})
    add_deps("DirectXMath", "vulkan", {public = true})
    add_packages(packages_list, {public = true})
    -- runtime compile definitions
    after_load(function (target,  opt)
        if (target:get("kind") == "shared") then
            import("core.project.project")
            local depgraph_lib = project.target("SkrDependencyGraph")
            depgraph_lib:add("defines", "EA_DLL", {public = true})

            target:add("defines", "MI_SHARED_LIB", "EA_DLL", "MARL_DLL", {public = true})
            target:add("defines", "MI_SHARED_LIB_EXPORT", "EASTL_API=EA_EXPORT", "EASTL_EASTDC_API=EA_EXPORT", "MARL_BUILDING_DLL")
        end
    end)
    -- link system libs/frameworks
    if (is_os("windows")) then 
        add_links("advapi32", "Shcore", "user32", "shell32", "Ole32", {public = true})
    end
    if (is_os("macosx")) then 
        add_mxflags(project_cxflags, project_mxflags, {public = true, force = true})
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "Metal", "IOKit", {public = true})
        add_files("src/**/build.*.m", "src/**/build.*.mm")
    end

    add_links(links_list, {public = true})
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
        elseif is_os("linux") then 
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
    if has_config("is_unix") then 
        add_syslinks("pthread")
    end 
    add_files("$(projectdir)/thirdparty/FiberTaskingLib/source/*.cpp")
    
    -- add marl source
    add_defines("MARL_USE_EASTL", {public = true})
    local marl_source_dir = "$(projectdir)/thirdparty/marl"
    add_files(marl_source_dir.."/src/**.cpp")
    if not has_config("is_msvc") then 
        add_files(marl_source_dir.."/src/**.c")
        add_files(marl_source_dir.."/src/**.S")
    end
    add_includedirs("$(projectdir)/thirdparty/marl/include", {public = true})
    add_includedirs("$(projectdir)/thirdparty/marl/include/marl", {public = true})

    -- install dxc on windows platform
    if (is_os("windows")) then 
        add_rules("utils.install-libs", { libnames = {"dxc"} })
    end

    -- cpu info private include dir
    add_includedirs("include/platform/cpu", {public = false})