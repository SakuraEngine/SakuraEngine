boost_includes_dir = "$(projectdir)/thirdparty/boost"

target("boost")
    set_group("00.thirdparty")
    set_kind("headeronly")
    add_includedirs(boost_includes_dir, {public=true})