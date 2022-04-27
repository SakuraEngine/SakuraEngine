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