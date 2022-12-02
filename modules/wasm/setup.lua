import("core.project.project")
module_root = path.join(project.directory(), "xmake/modules")
import("find_sdk", {rootdir = module_root})

if (os.host() == "windows") then
    find_sdk.lib_from_github("m3", "m3-windows-x64.zip")
    find_sdk.lib_from_github("m3_d", "m3_d-windows-x64.zip")
end

if (os.host() == "macosx") then
    if (os.arch() == "x86_64") then
        find_sdk.lib_from_github("m3", "m3-macosx-x86_64.zip")
        find_sdk.lib_from_github("m3_d", "m3_d-macosx-x86_64.zip")
    else
        find_sdk.lib_from_github("m3", "m3-macosx-arm64.zip")
        find_sdk.lib_from_github("m3_d", "m3_d-macosx-arm64.zip")
    end
end