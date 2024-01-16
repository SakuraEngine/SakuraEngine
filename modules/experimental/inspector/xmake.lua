 shared_module("SkrInspector", "SKR_INSPECT", engine_version)
    -- add_rules("c++.codegen", {
    --     files = {"include/**.h", "include/**.hpp"},
    --     rootdir = "include/SkrInspector",
    --     api = "SKR_INSPECT"
    -- })
    public_dependency("SkrDevCore", engine_version)
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")