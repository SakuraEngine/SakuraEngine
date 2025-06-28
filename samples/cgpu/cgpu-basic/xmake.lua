target("CGPUMandelbrot")
    set_group("04.examples/cgpu")
    set_kind("binary")
    set_exceptions("no-cxx")
    skr_unity_build()
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-mandelbrot",
        dxil_outdir = "/../resources/shaders/cgpu-mandelbrot"})
    public_dependency("SkrRT")
    add_deps("AppSampleCommon", "lodepng")
    add_files("mandelbrot/*.c")
    add_files("mandelbrot/**.hlsl")

target("CGPUIndexedInstance")
    set_group("04.examples/cgpu")
    set_kind("binary")
    set_exceptions("no-cxx")
    skr_unity_build()
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-indexed-instance",
        dxil_outdir = "/../resources/shaders/cgpu-indexed-instance"})
    public_dependency("SkrRT")
    add_deps("AppSampleCommon")
    add_files("indexed-instance/*.c")
    add_files("indexed-instance/**.hlsl")

target("CGPUTexture")
    set_group("04.examples/cgpu")
    set_kind("binary")
    set_exceptions("no-cxx")
    skr_unity_build()
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-texture",
        dxil_outdir = "/../resources/shaders/cgpu-texture"})
    public_dependency("SkrRT")
    add_deps("AppSampleCommon")
    add_files("texture/texture.c")
    add_files("texture/**.hlsl")

target("CGPUTiledTexture")
    set_group("04.examples/cgpu")
    set_exceptions("no-cxx")
    set_kind("binary")
    skr_unity_build()
    public_dependency("SkrRT")
    add_deps("AppSampleCommon")
    add_files("texture/tiled_texture.c")
