target("icu")
    set_group("00.thirdparty")
    set_kind("static")
    add_includedirs("icu4c/source/common", {public=true})
    add_includedirs("icu4c/source/i18n", {public=true})
    add_files("icu4c/source/common/**.cpp")
    add_files("icu4c/source/stubdata/**.cpp")
    add_defines("U_I18N_IMPLEMENTATION", {public=false})
    add_defines("U_COMMON_IMPLEMENTATION", {public=false})
    add_defines("U_STATIC_IMPLEMENTATION", {public=false})
    if (is_plat("windows")) then
        add_cxflags("/wd4267", "/wd4244", "/source-charset:utf-8", {public=false})
    end