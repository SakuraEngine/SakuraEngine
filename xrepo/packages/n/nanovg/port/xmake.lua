set_languages("c11", "cxx17")
add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")

target("nanovg")
    set_kind("static")
    set_optimize("fastest")
    add_headerfiles("nanovg/(**.h)")
    add_includedirs("nanovg", {public=true})
    add_files("nanovg/nanovg.cpp")