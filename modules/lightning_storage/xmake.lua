target("SkrLightningStorage")
    add_rules("skr.module", {api = "SKR_LIGHTNING_STORAGE"})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/", disable_reflection = true,
        api = "SKR_LIGHTNING_STORAGE"
    })
    add_deps("lmdb", "SkrRT")
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    -- add_files("src/**.c")