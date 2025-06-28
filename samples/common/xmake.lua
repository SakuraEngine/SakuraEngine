target("lodepng")
    set_group("04.examples/cgpu")
    set_kind("static")
    set_exceptions("no-cxx")
    add_deps("AppSampleCommon")
    add_files("lodepng/lodepng.c")
    add_includedirs("lodepng", {public = true})

target("AppSampleCommon")
    set_kind("headeronly")
    add_includedirs("include", {public = true})
    add_deps("SDL3", {public = true})
    add_deps("SkrCore")
    add_deps("SkrGraphics")

    -- for clangd
    add_files("_app_sample_dummy_src.cpp")