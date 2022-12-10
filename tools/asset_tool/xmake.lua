shared_module("SkrAssetTool", "SKR_ASSET_TOOL", engine_version)
    set_group("02.tools")
    public_dependency("SkrToolCore", engine_version)
    public_dependency("SkrTextureCompiler", engine_version)
    public_dependency("SkrShaderCompiler", engine_version)
    public_dependency("SkrGLTFTool", engine_version)
    public_dependency("SkrImGui", engine_version)
    public_dependency("GameTool", engine_version)
    public_dependency("SkrAnimTool", engine_version)
    add_files("src/**.cpp")
    add_includedirs("include", {public=true})

    if(has_config("build_usdtool")) then
        public_dependency("SkrUsdTool", engine_version)
        add_defines("WITH_USDTOOL", {public = false})
    end
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrAssetTool",
        api = "SKR_ASSET_TOOL"
    })
    set_exceptions("no-cxx")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})

executable_module("SkrAssetImport", "SKR_ASSET_IMPORT", engine_version)
    set_group("02.tools")
    public_dependency("SkrAssetTool", engine_version)
    add_files("main.cpp", "imgui.cpp", "imgui_impl_sdl.cpp")
    if(has_config("build_usdtool")) then
        public_dependency("SkrUsdTool", engine_version)
        add_defines("WITH_USDTOOL", {public = false})
    end
    set_exceptions("no-cxx")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})