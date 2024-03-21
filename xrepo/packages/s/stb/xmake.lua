package("stb")
    set_homepage("https://github.com/nothings/stb")
    set_description("stb single-file public domain libraries for C/C++.")
    set_license("MIT License")
    
    add_versions("2.29", "89b2b5ddb0f3f94b7b1994ed54300194cf974724963ea2f6718a50ef50a65974")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "stb"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)