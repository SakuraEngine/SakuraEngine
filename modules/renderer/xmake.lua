target("SkrRenderer")
    add_rules("c++.reflection", {
        files = {"game/**.h", "game/**.hpp"},
        rootdir = "game/"
    })
    set_kind("shared")
    add_deps("SkrRenderGraph")
    add_includedirs("include", {public=true})
    add_defines("SKR_RENDERER_SHARED", {public=true})
    add_defines("SKR_RENDERER_IMPL")
    add_files("src/*.cpp")