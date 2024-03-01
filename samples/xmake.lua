includes("application/xmake.lua")
includes("hotfix/xmake.lua")

if build_part("samples.cgpu") then
    includes("cgpu/xmake.lua")
end

if build_part("samples.render_graph") then
    includes("render_graph/xmake.lua")
end

if build_part("samples.AAA") then
    includes("AAA/xmake.lua")
end