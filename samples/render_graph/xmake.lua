add_requires("lmdb >=0.9.29-skr")

target("RenderGraphTriangle")
    set_group("04.examples/render_graph")
    set_kind("binary")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-triangle",
        dxil_outdir = "/../resources/shaders/rg-triangle"})
    set_exceptions("no-cxx")
    skr_unity_build()
    public_dependency("SkrRT")
    public_dependency("SkrRenderGraph")
    add_deps("AppSampleCommon")
    add_files("rg-triangle/*.cpp")
    add_files("rg-triangle/**.hlsl")

target("RenderGraphDeferred")
    set_group("04.examples/render_graph")
    set_kind("binary")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-deferred",
        dxil_outdir = "/../resources/shaders/rg-deferred"})
    set_exceptions("no-cxx")
    skr_unity_build()
    public_dependency("SkrRT")
    public_dependency("SkrRenderGraph")
    public_dependency("SkrImGui")
    add_deps("AppSampleCommon")
    add_files("rg-deferred/*.cpp")
    add_files("rg-deferred/**.hlsl")

target("RenderGraphCrossProcess")
    set_group("04.examples/render_graph")
    set_kind("binary")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cross-process",
        dxil_outdir = "/../resources/shaders/cross-process"})
    set_exceptions("no-cxx")
    skr_unity_build()
    public_dependency("SkrRT")
    public_dependency("SkrRenderGraph")
    public_dependency("SkrImGui")
    add_deps("AppSampleCommon")
    add_files("cross-process/*.cpp")
    add_files("cross-process/**.hlsl")
    add_packages("lmdb")
