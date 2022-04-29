if (os.host() =="macosx") then 
    import("lib.detect.find_tool")
    local brew = find_tool("brew")
    if(brew == nil) then
        os.runv("/bin/bash", {"-c", "\"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""})
    end
    os.exec("brew install sdl2")
    os.exec("brew install grpc")
    os.exec("brew install googletest")
end
import("find_sdk")
find_sdk.install_tool("dxc")
find_sdk.install_tool("reflector")
find_sdk.install_tool("wasm-clang")
find_sdk.install_tool("grpcc")