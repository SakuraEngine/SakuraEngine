add_requires("boost-context >=0.1.0-skr")

shared_module("SkrTask", "SKR_TASK", engine_version)
    -- add source files
    public_dependency("SkrCore", engine_version)
    add_includedirs("include", {public = true})
    add_files("src/build.*.cpp")
    -- internal packages
    add_packages("boost-context", {public = true, inherit = true})
    -- add FTL source 
    local ftl_includes_dir = "$(projectdir)/thirdparty/FiberTaskingLib/include"
    add_includedirs(ftl_includes_dir, {public = true})
    add_files("$(projectdir)/thirdparty/FiberTaskingLib/source/build.*.cpp")
    -- add marl source
    local marl_source_dir = "$(projectdir)/thirdparty/marl"
    add_files(marl_source_dir.."/src/build.*.cpp")
    if not is_os("windows") then 
        add_files(marl_source_dir.."/src/**.c")
        add_files(marl_source_dir.."/src/**.S")
    end
    add_includedirs("$(projectdir)/thirdparty/marl/include", {public = true})
    -- marl runtime compile definitions
    after_load(function (target,  opt)
        if (target:get("kind") == "shared") then
            import("core.project.project")
            target:add("defines", "MARL_DLL", {public = true})
            target:add("defines", "MARL_BUILDING_DLL")
        end
    end)