module_root = path.absolute("xmake/modules")
import("find_sdk", {rootdir = module_root})

if (os.host() =="macosx") then 
    import("lib.detect.find_tool")
    local brew = find_tool("brew")
    if(brew == nil) then
        os.runv("/bin/bash", {"-c", "\"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""})
    end
    os.exec("brew install ispc")
end

-- python
find_sdk.file_from_github("SourceSansPro-Regular.ttf")
if (os.host() == "windows") then
    find_sdk.tool_from_github("python-embed", "python-embed-windows-x64.zip")
else
    pip = find_sdk.find_program("pip3") or find_sdk.find_program("pip") or {program = "pip"}
    os.runv(pip.program, {"install", "mako"})
end

if (os.host() == "windows") then
    find_sdk.sdk_lib_from_github("lmdb", "lmdb-windows-x64.zip")
    find_sdk.sdk_lib_from_github("cgltf", "cgltf-windows-x64.zip")
    find_sdk.sdk_lib_from_github("fmt", "fmt-windows-x64.zip")
    find_sdk.sdk_lib_from_github("fmt_d", "fmt_d-windows-x64.zip")
    find_sdk.sdk_lib_from_github("imgui", "imgui-windows-x64.zip")
    find_sdk.sdk_lib_from_github("imgui_d", "imgui_d-windows-x64.zip")
    find_sdk.sdk_lib_from_github("simdjson", "simdjson-windows-x64.zip")
    find_sdk.sdk_lib_from_github("simdjson_d", "simdjson_d-windows-x64.zip")
    find_sdk.sdk_lib_from_github("ISPCTextureCompressor", "ISPCTextureCompressor-windows-x64.zip")

    -- gfx
    find_sdk.lib_from_github("WinPixEventRuntime", "WinPixEventRuntime-windows-x64.zip")
    find_sdk.lib_from_github("amdags", "amdags-windows-x64.zip")
    find_sdk.lib_from_github("nvapi", "nvapi-windows-x64.zip")
    find_sdk.lib_from_github("nsight", "nsight-windows-x64.zip")
    find_sdk.lib_from_github("dstorage", "dstorage-windows-x64.zip")
    find_sdk.lib_from_github("SDL2", "SDL2-windows-x64.zip")
    find_sdk.lib_from_github("tracyclient", "tracyclient-windows-x64.zip")
    find_sdk.lib_from_github("tracyclient_d", "tracyclient_d-windows-x64.zip")
    --
    find_sdk.lib_from_github("usd", "usd-windows-x64.zip")
    --
    find_sdk.tool_from_github("dxc", "dxc-windows-x64.zip")
    find_sdk.tool_from_github("wasm-clang", "wasm-clang-windows-x64.zip")
    find_sdk.tool_from_github("ispc", "ispc-windows-x64.zip")
    find_sdk.tool_from_github("reflector", "reflector-windows-x64.zip")
    find_sdk.tool_from_github("tracy-gui", "tracy-gui-windows-x64.zip")
end

if (os.host() == "macosx") then
    if (os.arch() == "x86_64") then
        -- 
        find_sdk.lib_from_github("tracyclient", "tracyclient-macosx-x86_64.zip")
        --
        find_sdk.tool_from_github("dxc", "dxc-macosx-x86_64.zip")
        find_sdk.tool_from_github("reflector", "reflector-macosx-x86_64.zip")
        find_sdk.tool_from_github("tracy-gui", "tracy-gui-macosx-x86_64.zip")
    else
        find_sdk.tool_from_github("dxc", "dxc-macosx-arm64.zip")
    end
end

local setups = os.files("**/setup.lua")
for _, setup in ipairs(setups) do
    local dir = path.directory(setup)
    local basename = path.basename(setup)
    import(path.join(dir, basename))
end

if (os.host() == "windows") then
    os.exec("xmake project -k vsxmake -m \"debug,release\" -a x64 -y")
end