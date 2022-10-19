shared_module("SkrInputSystem", "SKR_INPUTSYSTEM", engine_version)
    set_group("01.modules")
    add_rules("c++.codegen", {
        files = {"include/**.h"},
        rootdir = "include/",
        api = "SKR_INPUTSYSTEM"
    })
    public_dependency("SkrGAInput", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/*.cpp")
    
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
