add_requires("daScript 2023.4.24-skr.2")

target("daSTest0")
    set_kind("binary")
    set_group("05.tests/daS")
    public_dependency("SkrRT", engine_version)
    add_packages("gtest", "daScript")
    add_files("daSTest0/**.cpp")