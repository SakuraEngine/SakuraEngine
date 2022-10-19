target("GLTFTool")
    set_group("02.tools")
    add_rules("skr.module", {api = "GLTFTool", version = engine_version})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/"
    })
    public_dependency("SkrTool", engine_version)
    public_dependency("GameRT", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_rules("c++.noexception")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_deps("cgltf")