tracy_includes_dir = "$(projectdir)/thirdparty/tracy"
table.insert(include_dir_list, tracy_includes_dir)
table.insert(links_list, "TracyClient")

task("unzip-tracyclient")
    on_run(function ()
        import("find_sdk")
        find_sdk.install_lib("tracyclient")
    end)