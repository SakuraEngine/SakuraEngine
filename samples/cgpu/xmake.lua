target("Example-CGPUMandelbrot")
    set_group("04.examples/cgpu")
    add_rules("c++.noexception")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-mandelbrot",
        dxil_outdir = "/../resources/shaders/cgpu-mandelbrot"})
    set_kind("binary")
    add_deps("SkrRT")
    add_files("cgpu-mandelbrot/*.c")
    add_files("cgpu-mandelbrot/**.hlsl")

target("Example-CGPUIndexedInstance")
    set_group("04.examples/cgpu")
    add_rules("c++.noexception")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-indexed-instance",
        dxil_outdir = "/../resources/shaders/cgpu-indexed-instance"})
    set_kind("binary")
    add_deps("SkrRT")
    add_files("cgpu-indexed-instance/*.c")
    add_files("cgpu-indexed-instance/**.hlsl")

target("Example-CGPUTexture")
    set_group("04.examples/cgpu")
    add_rules("c++.noexception")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-texture",
        dxil_outdir = "/../resources/shaders/cgpu-texture"})
    set_kind("binary")
    add_deps("SkrRT")
    add_files("cgpu-texture/*.c")
    add_files("cgpu-texture/**.hlsl")
    

if (os.host() == "windows") then
    target("Example-HotTriangle")
        set_group("04.examples/cgpu")
        add_rules("utils.dxc", {
            spv_outdir = "/../resources/shaders/hot-triangle",
            dxil_outdir = "/../resources/shaders/hot-triangle"})
        set_kind("binary")
        -- file_watch.hpp needs exceptions
        -- add_rules("c++.exception")
        add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
        add_deps("SkrRT", "SkrWASM")
        add_files("hot-triangle/triangle.c", "hot-triangle/hot_wasm.cpp")
        add_files("hot-triangle/**.hlsl")
end

target("Example-CGPU3D")
    set_group("04.examples/cgpu")
    add_rules("c++.noexception")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_rules("utils.install-resources", {
        extensions = {".gltf", ".bin", ".png"},
        outdir = "/../resources", _png_outdir = "/../resources/textures"})
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-3d",
        dxil_outdir = "/../resources/shaders/cgpu-3d"})
    set_kind("binary")
    add_deps("cgltf", "SkrRT")
    add_files("cgpu-3d/**.cpp")
    add_files("cgpu-3d/**.hlsl")
    add_files("cgpu-3d/**.bin", "cgpu-3d/**.gltf", "cgpu-3d/**.png")