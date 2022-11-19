shared_module("SkrLightningStorage", "SKR_LIGHTNING_STORAGE", engine_version)
    set_group("01.modules")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/",
        api = "SKR_LIGHTNING_STORAGE"
    })
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    -- add_files("src/**.c")
    add_deps("lmdb")