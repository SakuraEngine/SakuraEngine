task("unzip-gfx-sdk")
    on_run(function ()
        import("utils.archive")
        if(os.host() == "windows") then
            zipped_ags = "amd_ags.zip"
            zipped_nvapi = "nvapi.zip"
            zipped_pix3 = "WinPixEventRuntime.zip"
            ags_sdkdir = path.join(os.projectdir(), "SDKs/"..zipped_ags)
            nvapi_sdkdir = path.join(os.projectdir(), "SDKs/"..zipped_nvapi)
            pix3_sdkdir = path.join(os.projectdir(), "SDKs/"..zipped_pix3)
            local outputdir = path.join(os.projectdir(), "build/"..os.host().."/"..os.arch().."/"..vformat("$(mode)"))
            archive.extract(ags_sdkdir, outputdir)
            archive.extract(nvapi_sdkdir, outputdir)
            archive.extract(pix3_sdkdir, outputdir)

            sdl2_sdkdir = path.join(os.projectdir(), "SDKs/SDL2-win.zip")
            archive.extract(sdl2_sdkdir, outputdir)
        end
        -- dxc
        if(os.host() == "windows") then
            zipped_dxc = "dxc-win.zip"
            dxcdir = path.join(os.projectdir(), "SDKs/"..zipped_dxc)
        else 
            zipped_dxc = "dxc-macos-amd64.zip"
            dxcdir = path.join(os.projectdir(), "SDKs/"..zipped_dxc)
        end
        local outputdir = path.join(os.projectdir(), vformat("$(buildir)/sdk"))
        print("unzip dxc "..zipped_dxc.." -> "..outputdir)
        archive.extract(dxcdir, outputdir)
        if(os.host() ~= "windows") then
            os.exec("chmod a+x "..outputdir.."/dxc")
        end
        -- reflector
        if(os.host() == "windows") then
            zipped_reflector = "reflector-win.zip"
            refldir = path.join(os.projectdir(), "SDKs/"..zipped_reflector)
        else 
            zipped_reflector = "reflector-macos-amd64.zip"
            refldir = path.join(os.projectdir(), "SDKs/"..zipped_reflector)
        end
        print("unzip reflector "..zipped_reflector.." -> "..outputdir)
        archive.extract(refldir, outputdir)
        if(os.host() ~= "windows") then
            os.exec("chmod a+x "..outputdir.."/meta")
        end
        -- grpc
        if(os.host() == "windows") then
            zipped_grpc = "grpc_r-win.zip"
            grpcdir = path.join(os.projectdir(), "SDKs/"..zipped_grpc)
        else 
            zipped_grpc = "grpc_r-macos-amd64.zip"
            grpcdir = path.join(os.projectdir(), "SDKs/"..zipped_grpc)
        end
        print("unzip grpc "..zipped_grpc.." -> "..path.join(os.projectdir(), "SDKs/"))
        archive.extract(grpcdir, path.join(os.projectdir(), "SDKs/"))
end)