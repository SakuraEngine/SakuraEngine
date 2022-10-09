target("SkrRenderer")
    set_group("01.modules")
    add_rules("skr.module", {api = "SKR_RENDERER"})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/",
        api = "SKR_RENDERER"
    })
    add_rules("c++.unity_build", {batchsize = 10})
    add_deps("cgltf", "SkrScene", "SkrRenderGraph", "SkrImGui")
    add_includedirs("include", {public=true})
    add_files("src/*.cpp")