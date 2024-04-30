includes("graphics/xmake.lua")
includes("image_coder/xmake.lua")
includes("runtime/xmake.lua")
includes("scene/xmake.lua")
includes("input/xmake.lua")
includes("input_system/xmake.lua")
includes("basic_meta/xmake.lua")

-- includes("lua/xmake.lua") -- FIXME. lua support

if build_part("v8") then 
    includes("v8/xmake.lua")
end