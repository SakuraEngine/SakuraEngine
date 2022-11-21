shared_module("SkrAnim", "SKR_ANIM", engine_version)
    set_group("01.modules")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrAnim",
        api = "SKR_ANIM"
    })
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrRenderer", engine_version)
    add_includedirs("include", {public=true})
    add_includedirs("ozz", {public=true})
    add_includedirs("src", {public=false})
    add_files("src/**.cpp", "src/**.cc")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})