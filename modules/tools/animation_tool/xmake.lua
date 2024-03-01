add_requires("cgltf >=1.13.0-skr", {system = false})
add_requires("tinygltf >=2.8.14-skr", {system = false})

shared_module("SkrAnimTool", "SKR_ANIMTOOL", engine_version)
    set_group("02.tools")
    set_exceptions("cxx")
    add_packages("cgltf", "tinygltf")
    public_dependency("SkrToolCore", engine_version)
    public_dependency("SkrGLTFTool", engine_version)
    public_dependency("SkrAnim", engine_version)
    add_includedirs("include", {public=true})
    add_includedirs("ozz", {public=true})
    add_includedirs("src", {public=false})

    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrAnimTool",
        api = "SKR_ANIMTOOL"
    })
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    set_pcxxheader("src/pch.hpp")

    add_files("src/*.cc", {unity_group = "utils"})
    add_files("src/tools/*.cc", "src/*.cpp", {unity_group = "tool"})
    add_files("src/gltf/**.cc", "src/gltf/**.cpp", {unity_ignored = false})