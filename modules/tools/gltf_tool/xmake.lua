add_requires("cgltf >=1.13.0-skr", {system = false})

codegen_component("SkrGLTFTool", { api = "GLTFTOOL", rootdir = "include/SkrGLTFTool" })
    add_files("include/**.h")
    add_files("include/**.hpp")

shared_module("SkrGLTFTool", "GLTFTOOL", engine_version)
    set_group("02.tools")
    add_packages("cgltf", {public=true})
    public_dependency("SkrMeshCore", engine_version)
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
            
private_pch("SkrGLTFTool")
    add_files("src/pch.hpp")