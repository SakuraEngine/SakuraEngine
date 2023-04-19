package("lua")
    set_homepage("https://github.com/lua/lua/")
    set_description("LUA language runtime.")
    set_license("Lua is certified Open Source software. [osi certified]Its license is simple and liberal and is compatible with GPL.")
    
    add_versions("5.4.4-skr", "dc5b3790c45e466583f29dcbc5be3e8d7964c5c5c302c53a27cabd3555378ebd")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "lua"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)