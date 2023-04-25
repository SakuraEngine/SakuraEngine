package("daScript")
    set_homepage("https://dascript.org/")
    set_description("daScript - high-performance statically strong typed scripting language")

    add_versions("2023.4.25-skr.1", "78b4c890839abadc3c8db801dbf2d57ca642a7f55edc9b654eafb633ffefad65")

    add_deps("daScriptCore 2023.4.25-skr.1")
    add_deps("daScriptTool 2023.4.25-skr.1")
    on_install(function (package)
        os.mkdir(package:installdir())
        os.mkdir(package:installdir("rules"))
        os.cp(path.join(package:scriptdir(), "port", "daScript", "daslib"), ".")
        os.cp(path.join(package:scriptdir(), "port", "daScript", "dastest"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")
        os.cp(path.join(package:scriptdir(), "rules", "AOT.lua"), "rules/AOT.lua")

        os.cp(path.join(package:scriptdir(), "port", "daScript", "daslib"), package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "daScript", "dastest"), package:installdir())

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)