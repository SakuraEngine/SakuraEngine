tracy_includes_dir = "$(projectdir)/thirdparty/tracy"
table.insert(include_dir_list, tracy_includes_dir)

task("unzip-tracyclient")
    on_run(function ()
        import("find_sdk")
        find_sdk.install_lib("tracyclient")
    end)