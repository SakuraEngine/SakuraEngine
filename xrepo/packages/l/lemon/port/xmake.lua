set_languages("c11", "cxx17")
add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")

target("lemon")
    set_kind("static")
    set_optimize("fastest")
    add_headerfiles("lemon/(**.h)")
    add_headerfiles("lemon/lemon/**.h", {prefixdir = "lemon"})
    add_includedirs("lemon")
    add_files(
        "lemon/lemon/arg_parser.cc",
        "lemon/lemon/base.cc",
        "lemon/lemon/color.cc",
        "lemon/lemon/lp_base.cc",
        "lemon/lemon/lp_skeleton.cc",
        "lemon/lemon/random.cc",
        "lemon/lemon/bits/windows.cc"
    )
    -- WIN32
    if is_plat("windows") then
        add_defines("WIN32")
        add_syslinks("user32")
    else
        add_syslinks("pthread")
    end