codegen_component("SkrShaderCompiler", { api = "SKR_SHADER_COMPILER", rootdir = "include/SkrShaderCompiler" })
    add_files("include/**.hpp")

shared_module("SkrShaderCompiler", "SKR_SHADER_COMPILER")
    set_group("02.tools")
    add_includedirs("include", {public=true})
    add_includedirs("src", {public=false})
    public_dependency("SkrRenderer")
    public_dependency("SkrToolCore")
    add_files("src/**.cpp")
    skr_unity_build()
    -- dxc compiler uses ms-extensions
    if (os.host() == "macosx") then
        add_cxflags("-fms-extensions", {public=false})
    end

    -- install
    skr_install_rule()
    skr_install("download", {
        name = "dxc-2025_07_14",
        install_func = "sdk",
    })

private_pch("SkrShaderCompiler")
    add_files("src/pch.hpp")