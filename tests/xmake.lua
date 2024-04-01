target("SkrTestFramework")
    set_kind("static")
    set_group("05.tests/framework")
    add_deps("SkrCore")
    add_includedirs("framework/include", {public = true})
    add_includedirs("framework/include/SkrTestFramework", {public = false})
    add_files("framework/src/framework.cpp")

-- includes("daS/xmake.lua")
if build_part("core") then
    includes("base/xmake.lua")
    includes("memory/xmake.lua")
end

if build_part("engine") then
    includes("cgpu/xmake.lua")
    includes("runtime/xmake.lua")
    includes("async/xmake.lua")
end
