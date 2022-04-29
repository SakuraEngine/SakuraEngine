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