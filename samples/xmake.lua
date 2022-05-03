set_project("Sakura.Samples")

target("rg-deferred")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-deferred",
        dxil_outdir = "/../resources/shaders/rg-deferred"})
    set_kind("binary")
    add_deps("SkrRT")
    add_files("rg-deferred/*.cpp")
    add_files("rg-deferred/**.hlsl")
    
target("cgpu-texture")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-texture",
        dxil_outdir = "/../resources/shaders/cgpu-texture"})
    set_kind("binary")
    add_deps("SkrRT")
    add_files("cgpu-texture/*.c")
    add_files("cgpu-texture/**.hlsl")

target("cgpu-3d")
    add_rules("utils.install-resources", {
        extensions = {".gltf", ".bin", ".png"},
        outdir = "/../resources", _png_outdir = "/../resources/textures"})
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-3d",
        dxil_outdir = "/../resources/shaders/cgpu-3d"})
    set_kind("binary")
    add_deps("SkrRT")
    add_files("cgpu-3d/**.cpp")
    add_files("cgpu-3d/**.hlsl")
    add_files("cgpu-3d/**.bin", "cgpu-3d/**.gltf", "cgpu-3d/**.png")

target("game")
    -- cxx reflection
    add_rules("c++.reflection", {
        files = {"game/**.h", "game/**.hpp"},
        rootdir = "game/"
    })
    set_kind("binary")
    add_deps("SkrRT")
    add_files("game/**.cpp")