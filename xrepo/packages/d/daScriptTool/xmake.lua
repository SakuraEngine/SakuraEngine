package("daScriptTool")
    set_kind("binary")
    set_homepage("https://dascript.org/")
    set_description("daScript - high-performance statically strong typed scripting language")

    add_versions("2023.4.25-skr", "233e949435ecc56669445bcce76d25823b915e2564ccf5b4f26514a561d82666")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "..", "daScriptCore", "port", "daScript", "include"), ".")
        os.cp(path.join(package:scriptdir(), "..", "daScriptCore", "port", "daScript", "src"), ".")
        os.cp(path.join(package:scriptdir(), "..", "daScriptCore", "port", "daScript", "test"), ".")
        os.cp(path.join(package:scriptdir(), "..", "daScriptCore", "port", "daScript", "3rdparty"), ".")
        os.cp(path.join(package:scriptdir(), "..", "daScriptCore", "port", "xmake.lua"), "xmakeDep.lua")

        os.cp(path.join(package:scriptdir(), "port", "daScript", "utils"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")
        os.cp(path.join(package:scriptdir(), "port", "da.daS"), package:installdir())

        local configs = {}
        import("package.tools.xmake").install(package, configs)

        os.cp("**/daScript", path.join(package:installdir(), "bin"))
        os.cp("**/daScript.*", path.join(package:installdir(), "bin"))
    end)

    on_test(function (package)
        os.vrun("daScript "..package:installdir().."/da.daS")
    end)


