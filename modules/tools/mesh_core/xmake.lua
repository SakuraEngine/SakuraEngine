add_requires("meshoptimizer >=0.1.0-skr")

codegen_component("SkrMeshCore", { api = "MESH_CORE", rootdir = "include/SkrMeshCore" })
    add_files("include/**.hpp")

shared_module("SkrMeshCore", "MESH_CORE", engine_version)
    set_group("02.tools")
    public_dependency("SkrToolCore", engine_version)
    public_dependency("SkrRenderer", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    -- meshoptimizer
    add_packages("meshoptimizer", {public=true})