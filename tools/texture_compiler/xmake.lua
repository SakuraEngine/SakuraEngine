target("ISPCTextureCompressor")
    set_group("02.tools")
    set_policy("build.across_targets_in_parallel", false)
    add_rules("utils.ispc")
    if (is_os("windows")) then 
        set_kind("headeronly")
        add_links(sdk_libs_dir.."ISPCTextureCompressor", {public=true} )
    else
        set_kind("static")
        add_files("src/**.ispc")
    end

shared_module("SkrTextureCompiler", "SKR_TEXTURE_COMPILER", engine_version)
    set_group("02.tools")
    add_includedirs("include", {public=true})
    public_dependency("SkrRenderer", engine_version)
    public_dependency("SkrImageCoder", engine_version)
    public_dependency("SkrToolCore", engine_version)
    add_deps("ISPCTextureCompressor")
    add_files("src/**.cpp")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrTextureCompiler",
        api = "SKR_TEXTURE_COMPILER"
    })
