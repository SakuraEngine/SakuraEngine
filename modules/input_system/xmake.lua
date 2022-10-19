target("SkrInputSystem")
    set_group("01.modules")
    add_rules("skr.module", {api = "SKR_INPUTSYSTEM", version = engine_version})
    add_rules("c++.codegen", {
        files = {"include/**.h"},
        rootdir = "include/",
        api = "SKR_INPUTSYSTEM"
    })
    public_dependency("SkrGAInput", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/*.cpp")
    
    add_rules("c++.noexception")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
