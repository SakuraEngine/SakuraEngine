wasm3_includes_dir = "$(projectdir)/thirdparty/wasm3"
table.insert(include_dir_list, wasm3_includes_dir)

task("unzip-wasm3")
on_run(function ()
    import("utils.archive")
    if(os.host() ~= "windows") then
        if is_mode("release") then
            zipped_wasm3 = "m3-macos-amd64.zip"
        else
            zipped_wasm3 = "m3_d-macos-amd64.zip"
        end
        sdkdir = path.join(os.projectdir(), "SDKs/"..zipped_wasm3)
    else 
        if is_mode("release") then
            zipped_wasm3 = "m3-win.zip"
        else
            zipped_wasm3 = "m3_d-win.zip"
        end
        sdkdir = path.join(os.projectdir(), "SDKs/"..zipped_wasm3)
    end
    local outputdir = path.join(os.projectdir(), "build/"..os.host().."/"..os.arch().."/"..vformat("$(mode)"))
    archive.extract(sdkdir, outputdir)
end)