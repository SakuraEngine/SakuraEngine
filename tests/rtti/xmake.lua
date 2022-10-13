target("rtti-test")
    set_group("05.tests")
    set_kind("binary")
    add_deps("SkrRT")
    add_files("rtti.cpp")
    add_packages("gtest")
    set_languages("c++17")
    add_rules("c++.codegen", {
        files = {"types/**.h", "types/**.hpp"},
        rootdir = "types/",
        api = "RTTI_TEST"
    })