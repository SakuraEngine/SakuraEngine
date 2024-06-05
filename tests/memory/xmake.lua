test_target("SSMTest")
    set_group("05.tests/memory")
    public_dependency("SkrCore", engine_version)
    add_files("SSM/*.cpp")