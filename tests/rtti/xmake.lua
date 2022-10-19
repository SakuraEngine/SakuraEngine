target("rtti-test-types")
    set_group("05.tests")
    add_rules("skr.module", {api = "RTTI_TEST_TYPES", version = engine_version})
    public_dependency("SkrRT", engine_version)
    add_files("types/types.cpp")
    set_languages("c++17")
    add_rules("c++.codegen", {
        files = {"types/**.h", "types/**.hpp"},
        rootdir = "types/",
        api = "RTTI_TEST_TYPES"
    })

target("rtti-test")
    set_group("05.tests")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("rtti-test-types", engine_version)
    add_files("rtti/rtti.cpp")
    add_packages("gtest")
    set_languages("c++17")
    add_rules("c++.codegen", {
        files = {"rtti/**.h", "rtti/**.hpp"},
        rootdir = "rtti/",
        api = "RTTI_TEST"
    })

target("sptr-test")
    set_group("05.tests")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("rtti-test-types", engine_version)
    add_files("sptr/**.cpp")
    add_packages("gtest")
    set_languages("c++17")
    add_rules("c++.codegen", {
        files = {"sptr/**.h", "sptr/**.hpp"},
        rootdir = "sptr/",
        api = "SPTR_TEST"
    })