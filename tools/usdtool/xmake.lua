shared_module("UsdTool", "USDTOOL", engine_version)
    set_group("02.tools")
    public_dependency("SkrTool", "0.1.0")
    public_dependency("GameRT", "0.1.0")
    add_deps("UsdCore")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/"
    })
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    -- enable exception
    if(has_config("is_msvc")) then
        add_cxflags("/EHsc")
        add_defines("_HAS_EXCEPTIONS=1")
    elseif(has_config("is_clang")) then
        add_cxflags("-fexceptions", "-fcxx-exceptions")
    end