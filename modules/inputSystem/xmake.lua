target("SkrInputSystem")
    add_rules("skr.module", {api = "SKR_INPUTSYSTEM"})
    add_rules("c++.reflection", {
        files = {"include/**.h"},
        rootdir = "include/",
        api = "SKR_INPUTSYSTEM"
    })
    add_deps("SkrGAInput")
    add_includedirs("include", {public=true})
    add_files("src/*.cpp")