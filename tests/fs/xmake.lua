add_requires("catch2 v3.4.0")

target("VFSTest")
    set_group("05.tests/base")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_packages("catch2")
    add_files("test/main.cpp")