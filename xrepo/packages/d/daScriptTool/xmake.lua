package("daScriptTool")
    set_kind("binary")
    set_homepage("https://dascript.org/")
    set_description("daScript - high-performance statically strong typed scripting language")

    add_versions("2023.4.25-skr.9", "dd65735b736d0cee9a9562c8e5bc1509c20055beb3ab05c5f1dc8c9b1de6189a")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.mkdir(package:installdir("bin"))
        os.cp(path.join(package:scriptdir(), "..", "daScriptCore", "port", "daScript", "include"), ".")
        os.cp(path.join(package:scriptdir(), "..", "daScriptCore", "port", "daScript", "src"), ".")
        os.cp(path.join(package:scriptdir(), "..", "daScriptCore", "port", "daScript", "test"), ".")
        os.cp(path.join(package:scriptdir(), "..", "daScriptCore", "port", "daScript", "3rdparty"), ".")
        os.cp(path.join(package:scriptdir(), "..", "daScriptCore", "port", "xmake.lua"), "xmakeDep.lua")

        os.cp(path.join(package:scriptdir(), "port", "daScript", "utils"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")
        os.cp(path.join(package:scriptdir(), "port", "da.daS"), package:installdir())

        local configs = {}
        configs.plat = os.host()
        configs.arch = os.arch()
        import("package.tools.xmake").install(package, configs)

        if is_host("windows") then
            os.cp("bin/daScript.exe", package:installdir("bin"))
        else
            os.cp("bin/daScript", package:installdir("bin"))
        end
    end)

    on_test(function (package)
        os.vrun("daScript "..package:installdir().."/da.daS")
    end)


