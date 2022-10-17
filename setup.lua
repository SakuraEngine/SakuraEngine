if (os.host() =="macosx") then 
    import("lib.detect.find_tool")
    local brew = find_tool("brew")
    if(brew == nil) then
        os.runv("/bin/bash", {"-c", "\"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""})
    end
    os.exec("brew install ispc")
    os.exec("brew install python")
    os.exec("brew install sdl2")
    os.exec("brew install googletest")
end
import("find_sdk")
pip = find_sdk.find_program("pip3") or find_sdk.find_program("pip") or {program = "pip"}
os.runv(pip.program, {"install", "mako"})

find_sdk.sdk_from_github("SourceSansPro-Regular.ttf")
if (os.host() == "windows") then
    find_sdk.sdk_from_github("wasm-clang-windows-x64.zip")
    -- gfx
    find_sdk.sdk_from_github("WinPixEventRuntime-windows-x64.zip")
    find_sdk.sdk_from_github("amdags-windows-x64.zip")
    find_sdk.sdk_from_github("dxc-windows-x64.zip")
    find_sdk.sdk_from_github("nvapi-windows-x64.zip")
    find_sdk.sdk_from_github("nsight-windows-x64.zip")
    find_sdk.sdk_from_github("dstorage-windows-x64.zip")
    --
    find_sdk.sdk_from_github("ispc-windows-x64.zip")
    find_sdk.sdk_from_github("reflector-windows-x64.zip")
    find_sdk.sdk_from_github("SDL2-windows-x64.zip")
    find_sdk.sdk_from_github("tracyclient-windows-x64.zip")
    find_sdk.sdk_from_github("tracyclient_d-windows-x64.zip")
    --
    find_sdk.sdk_from_github("usd-windows-x64.zip")
end

if (os.host() == "macosx") then
    if (os.arch() == "x86_64") then
        find_sdk.sdk_from_github("dxc-macosx-x86_64.zip")
        find_sdk.sdk_from_github("reflector-macosx-x86_64.zip")
        find_sdk.sdk_from_github("tracyclient-macosx-x86_64.zip")
    else
        -- find_sdk.sdk_from_github("ispc-macosx-arm64.zip")
        find_sdk.sdk_from_github("dxc-macosx-arm64.zip")
    end
end

find_sdk.install_tool("dxc")
find_sdk.install_tool("reflector")
find_sdk.install_tool("ispc")
if (os.host() == "windows") then
    find_sdk.install_tool("wasm-clang")
end