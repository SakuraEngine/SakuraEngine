package("nanovg")
    set_homepage("https://github.com/memononen/nanovg/")
    set_description("Antialiased 2D vector drawing library on top of OpenGL for UI and visualizations.")
    set_license("Zlib License")
    
    add_versions("0.1.0-skr", "16a19ecdd92ecd4ae4e6043862574a5288416cb5a6390011093832a070e6dac4")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "nanovg"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)