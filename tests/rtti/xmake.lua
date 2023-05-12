shared_module("RTTITestTypes", "RTTI_TEST_TYPES", engine_version)
    set_group("05.tests/rtti")
    public_dependency("SkrRT", engine_version)
    add_files("types/types.cpp")
    add_rules("c++.codegen", {
        files = {"types/**.h", "types/**.hpp"},
        rootdir = "types/",
        api = "RTTI_TEST_TYPES"
    })

target("RTTITest")
    set_group("05.tests/rtti")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("RTTITestTypes", engine_version)
    add_files("rtti/rtti.cpp")
    add_packages("gtest")
    add_rules("c++.codegen", {
        files = {"rtti/**.h", "rtti/**.hpp"},
        rootdir = "rtti/",
        api = "RTTI_TEST"
    })

target("SPtrTestCommon")
    set_group("05.tests/rtti")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("RTTITestTypes", engine_version)
    add_files("sptr/common.cpp")
    add_packages("gtest")

target("SPtrTestNonIntrusive")
    set_group("05.tests/rtti")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("RTTITestTypes", engine_version)
    add_files("sptr/non-intrusive.cpp")
    add_packages("gtest")

target("SPtrTestIntrusive")
    set_group("05.tests/rtti")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("RTTITestTypes", engine_version)
    add_files("sptr/intrusive.cpp")
    add_packages("gtest")
