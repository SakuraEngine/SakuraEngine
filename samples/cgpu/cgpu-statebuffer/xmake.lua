target("StateBufferTriangle")
    set_group("04.examples/cgpu")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/statebuffer-triangle",
        dxil_outdir = "/../resources/shaders/statebuffer-triangle"})
    set_kind("binary")
    -- file_watch.hpp needs exceptions
    set_exceptions("cxx")
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    public_dependency("SkrRT", engine_version)
    add_includedirs("./../../common", {public = false})
    add_files("triangle/statebuffer_triangle.c")
    add_files("triangle/shaders/**.hlsl")