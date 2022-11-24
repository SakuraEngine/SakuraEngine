shared_module("SkrUsdTool", "SKRUSDTOOL", engine_version)
    set_group("02.tools")
    public_dependency("SkrToolCore", "0.1.0")
    public_dependency("GameRT", "0.1.0")
    add_deps("UsdCore")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrUsdTool"
    })
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    -- enable exception
    set_exceptions("cxx")