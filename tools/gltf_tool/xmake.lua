shared_module("SkrGLTFTool", "GLTFTOOL", engine_version)
    set_group("02.tools")
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrGLTFTool",
        api = "GLTFTOOL"
    })
    public_dependency("SkrToolCore", engine_version)
    public_dependency("GameRuntime", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_deps("cgltf")

    -- add_files("lib/src/**.cpp", {unity_ignored = true})
    -- meshoptimizer
    if (is_os("windows")) then 
        add_linkdirs("lib/MeshOpt/windows", {public=true})
    elseif (is_os("macosx")) then 
        add_linkdirs("lib/MeshOpt/macos", {public=true})
    else
        add_files("lib/MeshOpt/src/**.cpp", {unity_ignored = true})
    end 
    add_links("MeshOptimizer", {public=true})