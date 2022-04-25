tracy_includes_dir = "$(projectdir)/thirdparty/tracy"
table.insert(include_dir_list, tracy_includes_dir)

task("unzip-tracyclient")
    on_run(function ()
        import("utils.archive")
        if(os.host() ~= "windows") then
            zipped_tracy = "tracyclient-macos-amd64.zip"
            sdkdir = path.join(os.projectdir(), "SDKs/"..zipped_tracy)
        else 
            if is_mode("release") then
                zipped_tracy = "tracyclient-win.zip"
            else
                zipped_tracy = "tracyclient_d-win.zip"
            end
            sdkdir = path.join(os.projectdir(), "SDKs/"..zipped_tracy)
        end
        local outputdir = path.join(os.projectdir(), vformat("$(buildir)/$(os)/$(arch)/$(mode)"))
        archive.extract(sdkdir, outputdir)
    end)