shared_module("RTTITestTypes", "RTTI_TEST_TYPES", engine_version)
    set_group("05.tests/framework")
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
    add_deps("SkrTestFramework", {public = false})
    add_packages("catch2", {public = true})
    add_rules("c++.codegen", {
        files = {"rtti/**.h", "rtti/**.hpp"},
        rootdir = "rtti/",
        api = "RTTI_TEST"
    })
    add_files("rtti/rtti.cpp")

target("SPtrTest")
    set_group("05.tests/sptr")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("RTTITestTypes", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_packages("catch2", {public = true})
    add_files("sptr/**.cpp")