target("rg-triangle")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-triangle",
        dxil_outdir = "/../resources/shaders/rg-triangle"})
    set_kind("binary")
    add_deps("SkrRT", "SkrRenderGraph")
    add_files("rg-triangle/*.cpp")
    add_files("rg-triangle/**.hlsl")

target("rg-deferred")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-deferred",
        dxil_outdir = "/../resources/shaders/rg-deferred"})
    set_kind("binary")
    add_deps("SkrRT", "SkrRenderGraph", "SkrImGui")
    add_files("rg-deferred/*.cpp")
    add_files("rg-deferred/**.hlsl")

target("cross-process")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cross-process",
        dxil_outdir = "/../resources/shaders/cross-process"})
    set_kind("binary")
    add_deps("lmdb", "SkrRT", "SkrRenderGraph", "SkrImGui")
    add_files("cross-process/*.cpp")
    add_files("cross-process/**.hlsl")