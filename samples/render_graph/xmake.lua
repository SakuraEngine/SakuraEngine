target("Example-RenderGraphTriangle")
    set_group("04.examples/render_graph")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-triangle",
        dxil_outdir = "/../resources/shaders/rg-triangle"})
    set_kind("binary")
    add_deps("SkrRT", "SkrRenderGraph")
    add_files("rg-triangle/*.cpp")
    add_files("rg-triangle/**.hlsl")

target("Example-RenderGraphDeferred")
    set_group("04.examples/render_graph")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-deferred",
        dxil_outdir = "/../resources/shaders/rg-deferred"})
    set_kind("binary")
    add_deps("SkrRT", "SkrRenderGraph", "SkrImGui")
    add_files("rg-deferred/*.cpp")
    add_files("rg-deferred/**.hlsl")

target("Example-RenderGraphCrossProcess")
    set_group("04.examples/render_graph")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cross-process",
        dxil_outdir = "/../resources/shaders/cross-process"})
    set_kind("binary")
    add_deps("lmdb", "SkrRT", "SkrRenderGraph", "SkrImGui")
    add_files("cross-process/*.cpp")
    add_files("cross-process/**.hlsl")