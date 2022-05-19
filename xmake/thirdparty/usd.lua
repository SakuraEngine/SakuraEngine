
task("unzip-usd")
    on_run(function ()
        import("find_sdk")
        find_sdk.install_lib("usd")
    end)