shared_module("SkrScene", "SKR_SCENE", engine_version)
    public_dependency("SkrRT", engine_version)
    -- TODO: add binding module
    public_dependency("SkrLua", engine_version)
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrScene",
        api = "SKR_SCENE"
    })
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_includedirs("include", {public=true})
    add_files("src/*.cpp")