shared_module("SkrOzz", "SKR_OZZ", engine_version)
    set_exceptions("no-cxx")
    set_optimize("fastest")
    public_dependency("SkrRT", engine_version)
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    set_pcxxheader("ozz_src/pch.hpp")
    add_includedirs("ozz", {public=true})
    add_includedirs("ozz_src", {public=false})
    add_files("ozz_src/**.cc")
    
shared_module("SkrAnim", "SKR_ANIM", engine_version)
    public_dependency("SkrOzz", engine_version)
    public_dependency("SkrRenderer", engine_version)
    add_includedirs("include", {public=true})
    add_includedirs("ozz", {public=true})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrAnim",
        api = "SKR_ANIM"
    })
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    set_pcxxheader("src/pch.hpp")
    add_files("src/**.cpp")