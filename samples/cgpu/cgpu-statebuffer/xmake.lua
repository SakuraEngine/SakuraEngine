target("StateBufferTriangle")
    set_group("04.examples/cgpu")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/statebuffer-triangle",
        dxil_outdir = "/../resources/shaders/statebuffer-triangle"})
    set_kind("binary")
    -- file_watch.hpp needs exceptions
    set_exceptions("no-cxx")
    skr_unity_build()
    public_dependency("SkrRT")
    add_deps("AppSampleCommon")
    add_files("triangle/statebuffer_triangle.c")
    add_files("triangle/shaders/**.hlsl")