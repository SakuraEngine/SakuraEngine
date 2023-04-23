add_requires("cgltf >=1.13.0-skr", {system = false})

shared_module("SkrGLTFTool", "GLTFTOOL", engine_version)
    set_group("02.tools")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrGLTFTool",
        api = "GLTFTOOL"
    })
    public_dependency("SkrMeshCore", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_packages("cgltf", {public=true})