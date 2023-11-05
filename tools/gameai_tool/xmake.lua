shared_module("SkrGameAINavTool", "SKR_GAMEAI_NAVTOOL", engine_version)
    set_group("02.tools")
    set_exceptions("cxx")
    add_packages("recastnavigation")
    public_dependency("SkrToolCore", engine_version)
    public_dependency("SkrGameAINav", engine_version)
    add_includedirs("include", {public=true})

    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrGameAINavTool",
        api = "SKR_GAMEAI_NAVTOOL"
    })
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_files("src/**.cpp", {unity_group = "tool"})