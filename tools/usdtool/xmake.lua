target("UsdTool")
    set_group("02.tools")

    if(has_config("is_msvc")) then
        add_cxflags("/EHsc")
        add_defines("_HAS_EXCEPTIONS=1")
    elseif(has_config("is_clang")) then
        add_cxflags("-fexceptions", "-fcxx-exceptions")
    end

    add_rules("skr.module", {api = "USDTOOL"})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/"
    })
    add_includedirs("include", {public=true})
    add_deps("SkrTool", "GameRT", "UsdCore")
    add_files("src/**.cpp")