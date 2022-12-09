if(not has_config("shipping_one_archive")) then

includes("tool_core/xmake.lua")

if(has_config("build_usdtool")) then
    includes("usdcore/xmake.lua")
    includes("usdtool/xmake.lua")
end 

includes("gltf_tool/xmake.lua")
includes("animation_tool/xmake.lua")
includes("shader_compiler/xmake.lua")
includes("texture_compiler/xmake.lua")
includes("resource_compiler/xmake.lua")
includes("asset_tool/xmake.lua")

end