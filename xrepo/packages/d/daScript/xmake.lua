package("daScript")
    set_homepage("https://dascript.org/")
    set_description("daScript - high-performance statically strong typed scripting language")

    add_versions("2023.4.26-skr.1", "2506e12bada34dd369d07fc1cf5e5b7ee18eb4869241838996e02a5e07ad1f66")

    add_deps("daScriptCore 2023.4.26-skr.1")
    add_deps("daScriptTool 2023.4.26-skr.1")
    
    on_install(function (package)
        os.mkdir(package:installdir())

        local dep = package:dep("daScriptTool")
        os.mkdir(package:installdir("bin"))
        if is_host("windows") then
            os.vcp(
                path.join(dep:installdir("bin"), "daScript.exe"), 
                path.join(package:installdir("bin"), "daScript.exe")
            )
            os.vcp(
                path.join(dep:installdir("bin"), "daScript.exe"), 
                path.join("bin", "daScript.exe")
            )
        else
            os.ln(
                path.join(dep:installdir("bin"), "daScript"), 
                path.join(package:installdir("bin"), "daScript")
            )
            os.ln(
                path.join(dep:installdir("bin"), "daScript"), 
                path.join("bin", "daScript")
            )
        end

        os.cp(path.join(package:scriptdir(), "rules"), ".")
        os.cp(path.join(package:scriptdir(), "modules"), ".")
        os.cp(path.join(package:scriptdir(), "port", "daScript", "daslib"), ".")
        os.cp(path.join(package:scriptdir(), "port", "daScript", "dastest"), ".")
        
        os.cp(path.join(package:scriptdir(), "modules"), package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "daScript", "daslib"), package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "daScript", "dastest"), package:installdir())

        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")
        os.cp(path.join(package:scriptdir(), "port", "da.daS"), package:installdir())

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)

    on_test(function (package)
        local cc = path.join(package:installdir("bin"), "daScript")
        os.vrun(cc.." "..package:installdir().."/da.daS")
    end)