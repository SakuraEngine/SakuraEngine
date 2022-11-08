includes("core/xmake.lua")

if(has_config("build_usdtool")) then
    includes("usdcore/xmake.lua")
    includes("usdtool/xmake.lua")
end 

includes("gltf_tool/xmake.lua")
includes("shader_compiler/xmake.lua")
includes("texture_compiler/xmake.lua")
includes("compiler/xmake.lua")