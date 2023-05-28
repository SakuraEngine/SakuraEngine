package("mimalloc")
    set_homepage("https://github.com/microsoft/mimalloc/")
    set_description("mimalloc is a compact general purpose allocator with excellent performance.")
    set_license("MIT License")
    
    add_versions("2.1.2", "D10C81DCEEFDB560A5E9BF5087307A7F293CC9FC44E3F7084FDF77D05D75A763")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "mimalloc"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)