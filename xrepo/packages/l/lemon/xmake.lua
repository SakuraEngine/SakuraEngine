package("lemon")
    set_homepage("https://lemon.cs.elte.hu/trac/lemon")
    set_description("Library for Efficient Modeling and Optimization in Networks.")
    set_license("BSL-1.0")

    add_versions("1.3.1", "71b7c725f4c0b4a8ccb92eb87b208701586cf7a96156ebd821ca3ed855bad3c8")
    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "lemon"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")
        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)

    on_test(function (package)
        assert(package:has_cxxtypes("lemon::SmartDigraph", {configs = {languages = "c++11"}, includes = "lemon/smart_graph.h"}))
    end)