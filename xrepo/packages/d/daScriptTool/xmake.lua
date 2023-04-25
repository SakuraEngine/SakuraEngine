package("daScriptTool")
    set_kind("binary")
    set_homepage("https://dascript.org/")
    set_description("daScript - high-performance statically strong typed scripting language")

    add_versions("2023.4.25-skr.1", "a86efedd01d456a0cab4c7819d9bae83356c2c94c221826e205893a53c49a594")

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
            os.cp("bin/daScript.exe", path.join(package:installdir(), "bin"))
        else
            os.cp("bin/daScript", path.join(package:installdir(), "bin"))
        end
    end)

    on_test(function (package)
        os.vrun("daScript "..package:installdir().."/da.daS")
    end)


