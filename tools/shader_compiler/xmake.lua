shared_module("SkrShaderCompiler", "SKR_SHADER_COMPILER", engine_version)
    set_group("02.tools")
    add_includedirs("include", {public=true})
    add_includedirs("src", {public=false})
    public_dependency("SkrRenderer", engine_version)
    public_dependency("SkrToolCore", engine_version)
    add_files("src/**.cpp")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrShaderCompiler",
        api = "SKR_SHADER_COMPILER"
    })
    -- dxc compiler uses ms-extensions
    if (os.host() == "macosx") then
        add_cxflags("-fms-extensions", {public=false})
    end
    -- install dxc on non-windows platform
    if (not is_os("windows")) then 
        add_rules("utils.install-libs", { libnames = {"dxc"} })
    end