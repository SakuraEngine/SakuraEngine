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
    find_sdk.sdk_lib_from_github("ISPCTextureCompressor", "ISPCTextureCompressor-windows-x64.zip")
    -- gfx sdk
    find_sdk.lib_from_github("WinPixEventRuntime", "WinPixEventRuntime-windows-x64.zip")
    find_sdk.lib_from_github("amdags", "amdags-windows-x64.zip")
    find_sdk.lib_from_github("nvapi", "nvapi-windows-x64.zip")
    find_sdk.lib_from_github("nsight", "nsight-windows-x64.zip")
    find_sdk.lib_from_github("dstorage-1.2.1", "dstorage-1.2.1-windows-x64.zip")
    find_sdk.lib_from_github("SDL2", "SDL2-windows-x64.zip")
    -- tools & compilers
    find_sdk.tool_from_github("dxc", "dxc-windows-x64.zip")
    find_sdk.tool_from_github("wasm-clang", "wasm-clang-windows-x64.zip")
    find_sdk.tool_from_github("ispc", "ispc-windows-x64.zip")
    find_sdk.tool_from_github("meta-v1.0.0-llvm_17.0.1", "meta-v1.0.0-llvm_17.0.1-windows-x64.zip")
    find_sdk.tool_from_github("tracy-gui-0.9.2a", "tracy-gui-0.9.2a-windows-x64.zip")
    -- network
    find_sdk.lib_from_github("gns", "gns-windows-x64.zip")
    find_sdk.lib_from_github("gns_d", "gns_d-windows-x64.zip")
end

if (os.host() == "macosx") then
    if (os.arch() == "x86_64") then
        --
        find_sdk.tool_from_github("dxc", "dxc-macosx-x86_64.zip")
        find_sdk.tool_from_github("meta-v1.0.0-llvm_17.0.1", "meta-v1.0.0-llvm_17.0.1-macosx-x86_64.zip")
        find_sdk.tool_from_github("tracy-gui-0.9.2a", "tracy-gui-0.9.2a-macosx-x86_64.zip")
        -- network
        find_sdk.lib_from_github("gns", "gns-macosx-x86_64.zip")
        find_sdk.lib_from_github("gns_d", "gns_d-macosx-x86_64.zip")
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

--[[
if (os.host() == "windows") then
    os.exec("xmake project -k vsxmake -m \"debug,release\" -a x64 -y")
end
]]--
