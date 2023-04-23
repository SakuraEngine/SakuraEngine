add_requires("lmdb =0.9.29-skr")

shared_module("SkrLightningStorage", "SKR_LIGHTNING_STORAGE", engine_version)
    set_group("01.modules")
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_packages("lmdb")