wasm3_includes_dir = "$(projectdir)/thirdparty/wasm3"
table.insert(include_dir_list, wasm3_includes_dir)

task("unzip-wasm3")
on_run(function ()
    import("find_sdk")
    find_sdk.install_lib("m3")
end)