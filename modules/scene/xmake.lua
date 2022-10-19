target("SkrScene")
    set_group("01.modules")
    add_rules("skr.module", {api = "SKR_SCENE", version = engine_version})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/",
        api = "SKR_SCENE"
    })
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/*.cpp")