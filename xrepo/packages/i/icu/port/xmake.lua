set_languages("c11", "cxx17")

target("icu")
    set_kind("static")
    set_optimize("fastest")
    add_includedirs("icu4c/source/common", {public=true})
    add_includedirs("icu4c/source/i18n", {public=true})
    add_headerfiles("icu4c/source/i18n/(**.h)", "icu4c/source/common/(**.h)")
    add_files("icu4c/source/i18n/**.cpp")
    add_files("icu4c/source/common/**.cpp")
    add_files("icu4c/source/stubdata/**.cpp")
    add_defines("U_I18N_IMPLEMENTATION", {public=false})
    add_defines("U_COMMON_IMPLEMENTATION", {public=false})
    add_defines("U_STATIC_IMPLEMENTATION", {public=false})
    if (is_plat("windows")) then
        add_cxflags("/wd4267", "/wd4244", "/source-charset:utf-8", {public=false})
    end