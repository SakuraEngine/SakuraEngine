gns_target_dir = "$(projectdir)/thirdparty/gamenetworkingsockets"
gns_include_dir = "$(projectdir)/thirdparty/gamenetworkingsockets/include"

target("gamenetworkingsockets")
    set_group("00.thirdparty")
    add_includedirs(gns_include_dir, {public = true})
    if is_os("windows") or is_os("macosx") then 
        add_rules("utils.install-libs", { libnames = {"gns"} })
        set_kind("headeronly")
        add_links("gamenetworkingsockets", {public=true} )
    else
        print("error: gamenetworkingsockets is not built on this platform!")
    end