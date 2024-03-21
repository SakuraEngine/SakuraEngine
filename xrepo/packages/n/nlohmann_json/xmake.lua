package("nlohmann_json")
    set_homepage("https://github.com/nlohmann/json")
    set_description("JSON for Modern C++.")
    set_license("MIT License")
    
    add_versions("3.11.3", "1b7944bde9d4fe9e86db98d575df2c7b3ee314a0f816de404a6b8a1e117b58dc")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "include"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            using json = nlohmann::json;
            void test() {
                json data;
                data["name"] = "world";
            }
        ]]}, {configs = {languages = "c++14"}, includes = {"nlohmann/json.hpp"}}))
    end)