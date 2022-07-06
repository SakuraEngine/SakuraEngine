target("SkrRenderer")
    add_rules("skr.module", {api = "SKR_RENDERER"})
    add_rules("c++.reflection", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/"
    })
    add_deps("SkrRenderGraph", "SkrImGui")
    add_includedirs("include", {public=true})
    add_files("src/*.cpp")