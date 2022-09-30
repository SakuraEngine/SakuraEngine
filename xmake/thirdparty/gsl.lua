gsl_includes_dir = "$(projectdir)/thirdparty/gsl"

target("gsl")
    set_group("00.thirdparty")
    set_kind("headeronly")
    add_includedirs(gsl_includes_dir, {public=true})