package("meshoptimizer")
    set_homepage("https://github.com/SakuraEngine/meshoptimizer/")
    set_description("This library provides algorithms to help optimize meshes for GPU stages, as well as algorithms to reduce the mesh complexity and storage overhead.")
    set_license("MIT License")
    
    add_versions("0.1.0-skr", "0d6f341738f30509262814b80fc9fdf5896d63eb4e81a0d755619edb34f3cec8")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "src"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)