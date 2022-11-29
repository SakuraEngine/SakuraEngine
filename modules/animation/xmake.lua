shared_module("SkrOzz", "SKR_OZZ", engine_version)
    set_group("01.modules")
    set_exceptions("no-cxx")
    add_rules("skr.static_module", {api = "SKR_DEPENDENCY_GRAPH"})
    set_optimize("fastest")
    add_includedirs("ozz", {public=true})
    add_includedirs("ozz/src", {public=false})
    add_files("ozz/src/**.cc")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    public_dependency("SkrRT", engine_version)
    
shared_module("SkrAnim", "SKR_ANIM", engine_version)
    set_group("01.modules")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrAnim",
        api = "SKR_ANIM"
    })
    public_dependency("SkrOzz", engine_version)
    public_dependency("SkrRenderer", engine_version)
    add_includedirs("include", {public=true})
    add_includedirs("ozz", {public=true})
    add_files("src/**.cpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})