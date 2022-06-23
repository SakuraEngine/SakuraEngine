set_project("Sakura.Samples")

target("rg-triangle")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-triangle",
        dxil_outdir = "/../resources/shaders/rg-triangle"})
    set_kind("binary")
    add_deps("SkrRT", "SkrRenderGraph")
    add_files("rg-triangle/*.cpp")
    add_files("rg-triangle/**.hlsl")

target("rg-deferred")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/rg-deferred",
        dxil_outdir = "/../resources/shaders/rg-deferred"})
    set_kind("binary")
    add_deps("SkrRT", "SkrRenderGraph", "SkrImGui")
    add_files("rg-deferred/*.cpp")
    add_files("rg-deferred/**.hlsl")

includes("cgpu/xmake.lua")
includes("application/xmake.lua")

if has_config("build_AAA") then 
    includes("AAA/xmake.lua")
end