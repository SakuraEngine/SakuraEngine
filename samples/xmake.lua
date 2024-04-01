includes("application/xmake.lua")

if build_part("engine") then
    includes("cgpu/xmake.lua")
    includes("hotfix/xmake.lua")
end

if build_part("render") then
    includes("render_graph/xmake.lua")
end

if build_part("dcc") then
    includes("dcc/xmake.lua")
end