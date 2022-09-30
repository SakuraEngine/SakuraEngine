dxmath_includes_dir = "$(projectdir)/thirdparty/DirectXMath/DirectXMath"
dxmath_unix_includes_dir = "$(projectdir)/thirdparty/DirectXMath/Unix"

target("DirectXMath")
    set_group("00.thirdparty")
    set_kind("headeronly")
    add_includedirs(dxmath_includes_dir, {public=true})
    if (is_os("windows") == nil) then 
        add_includedirs(dxmath_unix_includes_dir, {public=true})
    end