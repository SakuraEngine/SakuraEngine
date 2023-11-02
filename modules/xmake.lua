includes("profile/xmake.lua")
includes("base/xmake.lua")
includes("runtime/xmake.lua")
includes("runtime_exporter/xmake.lua")
includes("lightning_storage/xmake.lua")
includes("image_coder/xmake.lua")
includes("scene/xmake.lua")
includes("renderer/xmake.lua")
includes("render_graph/xmake.lua")
includes("imgui/xmake.lua")
includes("input/xmake.lua")
includes("input_system/xmake.lua")
includes("live2d/xmake.lua")
includes("animation/xmake.lua")
-- includes("inspector/xmake.lua") -- TODO. resume
includes("devtime/xmake.lua")
-- includes("tweak/xmake.lua") -- TODO. resume
includes("physics/xmake.lua")
includes("gui/xmake.lua") 
includes("netcode/xmake.lua")
-- includes("daScript/xmake.lua")

if has_config("build_cgpu_samples") then 
    includes("wasm/xmake.lua")
end