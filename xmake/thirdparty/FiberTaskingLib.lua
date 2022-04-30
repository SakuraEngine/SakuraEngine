ftl_includes_dir = "$(projectdir)/thirdparty/FiberTaskingLib/include"
boost_context_include_dir = "$(projectdir)/thirdparty/boost_context/include"

table.insert(include_dir_list, boost_context_include_dir)
table.insert(include_dir_list, ftl_includes_dir)
table.insert(deps_list, "ftl")

option("ftl_fiber_canary_bytes")
    set_default(false)
    set_showmenu(true)
    set_description("Enable canary bytes in fiber switching logic, to help debug errors")
option_end()

target("ftl")
    set_kind("static")
    add_options("pointer-size", "is_msvc", "ftl_fiber_canary_bytes", "is_unix")

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
    set_kind("static")
    if has_config("is_unix") then 
        add_syslinks("pthread")
    end 
    set_languages("c++17")
    add_includedirs(ftl_includes_dir, boost_context_include_dir)
    add_files("$(projectdir)/thirdparty/FiberTaskingLib/source/*.cpp")
    add_cxflags(project_cxflags, {public = true})