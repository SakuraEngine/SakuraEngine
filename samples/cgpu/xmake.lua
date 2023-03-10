if has_config("use_zig") then 
add_requires("zig 0.10.x")

target("Zig-CGPUMandelbrot")
    set_group("04.examples/cgpu")
    set_kind("binary")
    -- we use shaders compiled by "Example-CGPUMandelbrot" target
    -- add_files("cgpu-mandelbrot/**.hlsl")
    public_dependency("SkrRT", engine_version)
    set_toolchains("@zig", {zigcc = false})
    add_files("cgpu-mandelbrot/mandelbrot.zig")
    add_files("cgpu-mandelbrot/cgpu.zig")
end 

target("Example-CGPUMandelbrot")
    set_group("04.examples/cgpu")
    set_exceptions("no-cxx")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-mandelbrot",
        dxil_outdir = "/../resources/shaders/cgpu-mandelbrot"})
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_files("cgpu-mandelbrot/*.c")
    add_files("cgpu-mandelbrot/**.hlsl")

target("Example-CGPUIndexedInstance")
    set_group("04.examples/cgpu")
    set_exceptions("no-cxx")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-indexed-instance",
        dxil_outdir = "/../resources/shaders/cgpu-indexed-instance"})
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_files("cgpu-indexed-instance/*.c")
    add_files("cgpu-indexed-instance/**.hlsl")

target("Example-CGPUTexture")
    set_group("04.examples/cgpu")
    set_exceptions("no-cxx")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-texture",
        dxil_outdir = "/../resources/shaders/cgpu-texture"})
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_files("cgpu-texture/texture.c")
    add_files("cgpu-texture/**.hlsl")

target("Example-CGPUTexture2")
    set_group("04.examples/cgpu")
    set_exceptions("no-cxx")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_files("cgpu-texture/texture_2.c")
    
-- close this demo until we fix exception rule issue
if (os.host() == "windows" and false) then
    target("Example-HotTriangle")
        set_group("04.examples/cgpu")
        add_rules("utils.dxc", {
            spv_outdir = "/../resources/shaders/hot-triangle",
            dxil_outdir = "/../resources/shaders/hot-triangle"})
        set_kind("binary")
        -- file_watch.hpp needs exceptions
        set_exceptions("cxx")
        add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
        public_dependency("SkrRT", engine_version)
        public_dependency("SkrWASM", engine_version)
        add_files("hot-triangle/triangle.c", "hot-triangle/hot_wasm.cpp")
        add_files("hot-triangle/**.hlsl")
end