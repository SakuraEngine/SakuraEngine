if has_config("build_cgpu_samples") then 
    includes("cgpu/xmake.lua")
end

if has_config("build_rg_samples") then 
    includes("render_graph/xmake.lua")
end

includes("application/xmake.lua")

if has_config("build_AAA") then 
    includes("AAA/xmake.lua")
end

if(has_config("build_editors")) then
    includes("editor/xmake.lua")
end