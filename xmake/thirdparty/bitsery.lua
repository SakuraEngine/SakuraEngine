bitsery_includes_dir = "$(projectdir)/thirdparty/bitsery/include"

target("bitsery")
    set_group("00.thirdparty")
    set_kind("headeronly")
    add_includedirs(bitsery_includes_dir, {public=true})
    add_headerfiles(bitsery_includes_dir.."/**.h")