bitsery_includes_dir = "$(projectdir)/thirdparty/bitsery/include"

target("bitsery")
    set_kind("headeronly")
    add_includedirs(bitsery_includes_dir, {public=true})