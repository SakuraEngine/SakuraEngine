package("cpu_features")
    set_homepage("https://github.com/google/cpu_features")
    set_description("A cross platform C99 library to get cpu features at runtime.")
    set_license("Apache-2.0 License")
    
    add_versions("v0.9.0", "3EC9A121AD1AD9CAF9448BC8C6B198F99B154F60BB3981C8BCE724436B66865B")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "cpu_features"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)