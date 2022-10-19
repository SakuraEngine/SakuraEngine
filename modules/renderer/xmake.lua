target("SkrRenderer")
    set_group("01.modules")
    add_rules("skr.module", {api = "SKR_RENDERER", version = engine_version})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/",
        api = "SKR_RENDERER"
    })
    public_dependency("SkrScene", engine_version)
    public_dependency("SkrRenderGraph", engine_version)
    public_dependency("SkrImGui", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/*.cpp")
    add_rules("c++.noexception")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    -- link to gltf
    add_deps("cgltf")
