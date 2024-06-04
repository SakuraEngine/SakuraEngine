codegen_component("SkrShaderCompiler", { api = "SKR_SHADER_COMPILER", rootdir = "include/SkrShaderCompiler" })
    add_files("include/**.h")
    add_files("include/**.hpp")

shared_module("SkrShaderCompiler", "SKR_SHADER_COMPILER", engine_version)
    set_group("02.tools")
    add_includedirs("include", {public=true})
    add_includedirs("src", {public=false})
    public_dependency("SkrRenderer", engine_version)
    public_dependency("SkrToolCore", engine_version)
    add_files("src/**.cpp")
    set_pcxxheader("src/pch.hpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch})

    -- dxc compiler uses ms-extensions
    if (os.host() == "macosx") then
        add_cxflags("-fms-extensions", {public=false})
    end
    -- install dxc on non-windows platform
    if (not is_os("windows")) then 
        add_rules("utils.install-libs", { libnames = {"dxc"} })
    end