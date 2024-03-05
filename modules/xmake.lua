if build_part("core") then
    includes("core/xmake.lua")
end

if build_part("engine") then
    includes("engine/xmake.lua")
end

if build_part("render") then
    includes("render/xmake.lua")
end 

if build_part("devtime") then
    includes("devtime/xmake.lua")
end

if build_part("gui") then
    includes("gui/xmake.lua") 
end

-- includes("experimental/netcode/xmake.lua")
-- includes("experimental/physics/xmake.lua")
-- includes("experimental/tweak/xmake.lua") -- TODO. resume
-- includes("experimental/inspector/xmake.lua") -- TODO. resume
-- includes("experimental/runtime_exporter/xmake.lua")
-- includes("experimental/daScript/xmake.lua")

if build_part("dcc") then
    includes("dcc/xmake.lua")
end

if build_part("tools") then
    includes("tools/xmake.lua")
end

if build_part("samples.wasm") then
    includes("experimental/wasm/xmake.lua")
end