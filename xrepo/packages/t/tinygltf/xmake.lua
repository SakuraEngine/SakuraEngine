package("tinygltf")
    set_homepage("https://github.com/syoyo/tinygltf/")
    set_description("C++11 tiny glTF 2.0 library.")
    set_license("MIT License")
    
    add_versions("2.8.14-skr", "0d6f341738f30509262814b80fc9fdf5896d63eb4e81a0d755619edb34f3cec8")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "src"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)