package("boost-context")
    set_homepage("https://www.boost.org/")
    set_description("Boost Context Library.")
    set_license("Boost Software License")
    
    add_versions("0.1.0-skr", "2703233940ac43229233071d7de1f335cd5bf4a503e3d4ae2dacd31d8d45417c")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "boost_context"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}        
        import("package.tools.xmake").install(package, configs)
    end)

    --on_test(function (package)
    --end)