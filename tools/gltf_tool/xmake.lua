add_requires("cgltf >=1.13.0-skr", {system = false})

shared_module("SkrGLTFTool", "GLTFTOOL", engine_version)
    set_group("02.tools")
    add_packages("cgltf", {public=true})
    public_dependency("SkrMeshCore", engine_version)
    
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrGLTFTool",
        api = "GLTFTOOL"
    })
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    set_pcxxheader("src/pch.hpp")

    add_includedirs("include", {public=true})
    add_files("src/**.cpp")