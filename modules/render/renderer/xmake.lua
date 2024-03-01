shared_module("SkrRenderer", "SKR_RENDERER", engine_version)
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrRenderer",
        api = "SKR_RENDERER"
    })
    public_dependency("SkrScene", engine_version)
    public_dependency("SkrRenderGraph", engine_version)
    public_dependency("SkrImGui", engine_version)
    add_includedirs("include", {public=true})
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    set_pcxxheader("src/pch.hpp")
    add_files("src/*.cpp")
    add_files("src/resources/*.cpp", {unity_group = "resources"})