set_project("SakuraTool")

target("SkrTool")
    set_kind("static")
    add_files("core/src/**.cpp")
    add_deps("SkrRT")
    add_includedirs("core/include", {public = true})
    add_rules("c++.reflection", {
        files = {"core/**.h", "core/**.hpp"},
        rootdir = "core/"
    })