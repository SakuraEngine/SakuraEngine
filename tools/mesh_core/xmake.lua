add_requires("meshoptimizer >=0.1.0-skr")

shared_module("SkrMeshCore", "MESH_CORE", engine_version)
    set_group("02.tools")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrMeshCore",
        api = "MESH_CORE"
    })
    public_dependency("SkrToolCore", engine_version)
    public_dependency("GameRuntime", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    -- meshoptimizer
    add_packages("meshoptimizer", {public=true})