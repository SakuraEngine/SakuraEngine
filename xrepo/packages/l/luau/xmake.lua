package("luau")
    set_homepage("https://luau-lang.org/")
    set_description("A fast, small, safe, gradually typed embeddable scripting language derived from Lua.")
    set_license("MIT")
    add_versions("0.613.1", "27965c7c0a3e53ea92e7a51892b94acf12472268")
    on_install("linux", "windows", "mingw|x86_64", "macosx", function(package)
        -- install with xmake
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "luau"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)

        -- need to link with Ast last
        local libs = { "Luau.VM", "Luau.Compiler", "Luau.Ast" }
        for _, link in ipairs(libs) do
            package:add("links", link)
        end
    end)

    on_test(function(package)
        assert(package:check_cxxsnippets({ test = [[
            extern "C" {
            #include <lua.h>
            #include <luacode.h>
            #include <lualib.h>
            }

            void test() {
                auto L = luaL_newstate();
                luaL_openlibs(L);
                lua_close(L);
            }
        ]]}))
    end)
