package("parallel-hashmap")
    set_kind("library", {headeronly = true})
    set_homepage("https://greg7mdp.github.io/parallel-hashmap/")
    set_description("A family of header-only, very fast and memory-friendly hashmap and btree containers.")
    set_license("Apache-2.0")

    add_versions("1.3.4-skr", "656f2ec5a39ad6aef8635ae78cd1c10325a5e1a37b53983b78999b8973bb0bf1")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "parallel_hashmap"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)

    on_test(function(package)
        assert(package:check_cxxsnippets({
            test = [[
              #include <iostream>
              #include <string>
              #include <parallel_hashmap/phmap.h>

              using phmap::flat_hash_map;
              static void test() {
                flat_hash_map<std::string, std::string> email = {
                    { "tom",  "tom@gmail.com"},
                    { "jeff", "jk@gmail.com"},
                    { "jim",  "jimg@microsoft.com"}
                };
                for (const auto& n : email)
                    std::cout << n.first << "'s email is: " << n.second << "\n";
                email["bill"] = "bg@whatever.com";
                std::cout << "bill's email is: " << email["bill"] << "\n";
              }
            ]]
        }, {configs = {languages = "c++11"}}))
    end)