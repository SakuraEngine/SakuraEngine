package("daScript")
    set_homepage("https://dascript.org/")
    set_description("daScript - high-performance statically strong typed scripting language")

    add_versions("2023.4.24-skr.30", "2e4e949435ecc56669445bcce76d25823b915e2564ccf5b4f26514a561d85561")

    add_deps("daScriptCore 2023.4.24-skr.30")
    add_deps("daScriptTool 2023.4.24-skr.30")
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