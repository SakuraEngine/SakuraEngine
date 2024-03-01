includes("core/xmake.lua")
includes("engine/xmake.lua")
includes("render/xmake.lua")
includes("devtime/xmake.lua")
includes("gui/xmake.lua") 

-- includes("experimental/netcode/xmake.lua")
-- includes("experimental/physics/xmake.lua")
-- includes("experimental/tweak/xmake.lua") -- TODO. resume
-- includes("experimental/inspector/xmake.lua") -- TODO. resume
-- includes("experimental/runtime_exporter/xmake.lua")
-- includes("experimental/daScript/xmake.lua")

if build_part("samples.wasm") then
    includes("experimental/wasm/xmake.lua")
end