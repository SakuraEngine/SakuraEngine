package("cgltf")
    set_homepage("https://github.com/jkuhlmann/cgltf/")
    set_description("Single-file/stb-style C glTF loader and writer")
    set_license("MIT License")
    
    add_versions("1.13.0-skr", "aaa7f309efdc5b964e63576489d79f767fb06b7e5e6907dc3ff7001c62f053cd")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "cgltf"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)