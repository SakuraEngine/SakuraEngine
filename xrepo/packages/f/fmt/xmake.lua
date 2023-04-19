package("fmt")
    set_homepage("https://github.com/fmtlib/fmt/")
    set_description("{fmt} is an open-source formatting library providing a fast and safe alternative to C stdio and C++ iostreams.")
    set_license("FMT License. Details follow: https://github.com/fmtlib/fmt/blob/master/LICENSE.rst")
    
    add_versions("9.1.0-skr", "60066a2a1f74c549874d9a06c6fd8372f358dc1234ece2ebef189734b969b854")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "fmt"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)