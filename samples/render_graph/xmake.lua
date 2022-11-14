target("Example-RenderGraphTriangle")
    set_group("04.examples/render_graph")
    set_kind("binary")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-triangle",
        dxil_outdir = "/../resources/shaders/rg-triangle"})
    set_exceptions("no-cxx")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrRenderGraph", engine_version)
    add_files("rg-triangle/*.cpp")
    add_files("rg-triangle/**.hlsl")

target("Example-RenderGraphDeferred")
    set_group("04.examples/render_graph")
    set_kind("binary")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-deferred",
        dxil_outdir = "/../resources/shaders/rg-deferred"})
    set_exceptions("no-cxx")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrRenderGraph", engine_version)
    public_dependency("SkrImGui", engine_version)
    add_files("rg-deferred/*.cpp")
    add_files("rg-deferred/**.hlsl")

target("Example-RenderGraphCrossProcess")
    set_group("04.examples/render_graph")
    set_kind("binary")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cross-process",
        dxil_outdir = "/../resources/shaders/cross-process"})
    set_exceptions("no-cxx")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrRenderGraph", engine_version)
    public_dependency("SkrImGui", engine_version)
    add_files("cross-process/*.cpp")
    add_files("cross-process/**.hlsl")
    add_deps("lmdb")
