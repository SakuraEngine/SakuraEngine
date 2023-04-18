package("simdjson")
    set_homepage("https://github.com/simdjson/simdjson/")
    set_description("simdjson : Parsing gigabytes of JSON per second.")
    set_license("Apache-2.0 license")
    
    add_versions("3.0.0-skr", "def38bf7967c94620b2185386821e381fe1c3083fd3c6f3a4a756c6fc0174a5f")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "simdjson"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)