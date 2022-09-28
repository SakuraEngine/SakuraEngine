gsl_includes_dir = "$(projectdir)/thirdparty/gsl"

target("gsl")
    set_kind("headeronly")
    add_includedirs(gsl_includes_dir, {public=true})