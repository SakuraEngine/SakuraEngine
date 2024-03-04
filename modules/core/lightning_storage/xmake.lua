add_requires("lmdb >=0.9.29-skr")

shared_module("SkrLightningStorage", "SKR_LIGHTNING_STORAGE", engine_version)
    public_dependency("SkrCore", engine_version)
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_packages("lmdb")
    if is_plat("windows") then
        add_syslinks("Advapi32") -- mdb_env_setup_locks
    end