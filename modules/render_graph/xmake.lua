target("SkrRenderGraph")
    set_kind("shared")
    add_deps("SkrRT")
    add_includedirs("include", {public=true})
    add_defines("SKR_RENDER_GRAPH_SHARED", {public=true})
    add_defines("SKR_RENDER_GRAPH_IMPL")
    add_files("src/*.cpp")
    -- add render graph shaders
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders", 
        dxil_outdir = "/../resources/shaders"})
    add_files("shaders/*.hlsl")