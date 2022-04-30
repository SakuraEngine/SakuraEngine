dxmath_includes_dir = "$(projectdir)/thirdparty/DirectXMath/DirectXMath"
if (is_os("windows") == nil) then 
    table.insert(include_dir_list, "$(projectdir)/thirdparty/DirectXMath/Unix")
end

table.insert(include_dir_list, dxmath_includes_dir)