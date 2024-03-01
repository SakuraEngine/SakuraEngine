target("ISPCTextureCompressor")
    set_group("02.tools")
    set_policy("build.across_targets_in_parallel", false)
    set_kind("static")
    add_packages("ispc")
    add_rules("utils.ispc")
    add_files("src/**.ispc")

shared_module("SkrTextureCompiler", "SKR_TEXTURE_COMPILER", engine_version)
    set_group("02.tools")
    add_deps("ISPCTextureCompressor")
    public_dependency("SkrRenderer", engine_version)
    public_dependency("SkrImageCoder", engine_version)
    public_dependency("SkrToolCore", engine_version)
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrTextureCompiler",
        api = "SKR_TEXTURE_COMPILER"
    })
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")