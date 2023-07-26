add_requires("lmdb >=0.9.29-skr")

shared_module("SkrLightningStorage", "SKR_LIGHTNING_STORAGE", engine_version)
    set_group("01.modules")
    public_dependency("SkrRT", engine_version)
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_packages("lmdb")