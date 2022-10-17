target("GLTFTool")
    set_group("02.tools")
    add_rules("skr.module", {api = "GLTFTool"})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/"
    })
    add_rules("c++.noexception")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_includedirs("include", {public=true})
    add_deps("cgltf", "SkrTool", "GameRT")
    add_files("src/**.cpp")