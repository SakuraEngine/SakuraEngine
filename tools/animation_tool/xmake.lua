shared_module("SkrAnimTool", "SKR_ANIMTOOL", engine_version)
    set_group("02.tools")
    set_exceptions("cxx")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrAnimTool",
        api = "SKR_ANIMTOOL"
    })
    public_dependency("SkrToolCore", engine_version)
    public_dependency("SkrAnim", engine_version)
    add_includedirs("include", {public=true})
    add_includedirs("ozz", {public=true})
    add_includedirs("src", {public=false})

    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_files("src/*.cc", {unity_group = "utils"})
    add_files("src/tools/*.cc", {unity_group = "tool"})
    add_files("src/tools/jsoncpp/**.cpp", {unity_group = "jsoncpp"})
    add_files("src/gltf/**.cpp", "src/gltf/**.cc", {unity_group = "gltf"})