shared_module("SkrToolCore", "TOOL_CORE", engine_version)
    set_group("02.tools")
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_files("src/**.cpp")
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public = true})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrToolCore",
        api = "TOOL_CORE"
    })
    on_config(function (target, opt)
        target:add("defines", "SKR_RESOURCE_PLATFORM=u8\""..target:plat().."\"")
    end)

private_pch("SkrToolCore")
    add_files("src/pch.hpp")