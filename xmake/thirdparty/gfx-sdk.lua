task("unzip-gfx-sdk")
    on_run(function ()
        import("find_sdk")
        if(os.host() == "windows") then
            find_sdk.install_lib("amdags")
            find_sdk.install_lib("nvapi")
            find_sdk.install_lib("WinPixEventRuntime")
            find_sdk.install_lib("SDL2")
        end
        find_sdk.install_lib("grpc")
        find_sdk.install_lib("llfio")
    end)