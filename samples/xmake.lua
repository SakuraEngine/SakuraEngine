set_project("Sakura.Samples")

includes("cgpu/xmake.lua")
includes("render_graph/xmake.lua")
includes("application/xmake.lua")

if has_config("build_AAA") then 
    includes("AAA/xmake.lua")
end