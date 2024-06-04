codegen_component("SkrScene", { api = "SKR_SCENE", rootdir = "include/SkrScene" })
    add_files("include/**.h")


shared_module("SkrScene", "SKR_SCENE", engine_version)
    public_dependency("SkrRT", engine_version)
    -- public_dependency("SkrLua", engine_version) -- FIXME. lua support
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_includedirs("include", {public=true})
    add_files("src/*.cpp")