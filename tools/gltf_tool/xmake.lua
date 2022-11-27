shared_module("SkrGLTFTool", "GLTFTOOL", engine_version)
    set_group("02.tools")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrGLTFTool",
        api = "GLTFTOOL"
    })
    public_dependency("SkrToolCore", engine_version)
    public_dependency("GameRuntime", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_deps("cgltf")