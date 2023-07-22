add_requires("catch2 v3.4.0")

target("SkrTestFramework")
    set_kind("static")
    set_group("05.tests/framework")
    add_packages("catch2", {public = true, inherit = true})
    add_includedirs("framework/include", {public = true})
    add_files("framework/src/*.cpp")
    add_deps("SkrRT")

-- includes("daS/xmake.lua")
includes("cgpu/xmake.lua")
includes("runtime/xmake.lua")
includes("async/xmake.lua")