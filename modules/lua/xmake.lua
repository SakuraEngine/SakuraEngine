add_requires("luau", { configs = { extern_c = true }})

shared_module("SkrLua", "SKR_LUA", engine_version)
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrImGui", engine_version)
    add_packages("luau", {public = true, inherit = false})
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
