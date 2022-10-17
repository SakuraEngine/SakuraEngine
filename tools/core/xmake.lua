target("SkrTool")
    set_group("02.tools")
    add_rules("skr.module", {api = "TOOL"})
    add_files("src/**.cpp")
    add_deps("SkrRT")
    add_includedirs("include", {public = true})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/",
        api = "TOOL"
    })
    add_rules("c++.noexception")
    -- add_rules("c++.unity_build", {batchsize = default_unity_batch_size})