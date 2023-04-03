target("Example-StateStreamTriangle")
    set_group("04.examples/cgpu")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/statestream-triangle",
        dxil_outdir = "/../resources/shaders/statestream-triangle"})
    set_kind("binary")
    -- file_watch.hpp needs exceptions
    set_exceptions("cxx")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    public_dependency("SkrRT", engine_version)
    add_includedirs("./../../common", {public = false})
    add_files("triangle/statestream_triangle.c")
    add_files("triangle/shaders/**.hlsl")