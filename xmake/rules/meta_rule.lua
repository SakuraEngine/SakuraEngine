target("SkrMetaCodegenPolicy")
    set_kind("headeronly")
    set_group("00.utilities")
    -- dispatch codegen task
    before_build(function(target)
        import("meta_system")
        meta_system()
    end)

--[[ rule options
{
    files = { "include/**.h, include/**.hpp" }  -- files need reflection
    rootdir = "include/" -- root directory, codegen files will keep the same directory structure relative to this directory
    api = "SKR_XXXX_API" -- export api, used for export generated api
    use_new_framework = true, -- TODO. 过渡用，后面删
}
]]
rule("c++.codegen") -- TODO. use new name
    on_load(function (target, opt)
        -- add dependency
        target:add("deps", "SkrMetaCodegenPolicy")

        -- config
        local codegen_dir = path.join(target:autogendir({root = true}), target:plat(), "codegen")
        local source_file = path.join(codegen_dir, target:name(), "/generated.cpp")

        -- check generated files
        if not os.exists(source_file) then
            local gen_file = io.open(source_file, "w")                
            gen_file:print("static_assert(false, \"codegen of module "..target:name().." is not completed!\")")
            gen_file:close()
        end

        -- add to target configure
        target:add("files", source_file, { unity_ignored = true })
        target:add("includedirs", codegen_dir, {public = true})
    end)
rule_end()

--[[ rule options
{
    scripts = {
        {
            file = "script.py", -- script file
            private = false, -- if true, generated files will not be exported
        }
    },
    dep_files = { "test/*.py", "test/*.mako" }, -- dep_files
    ...
}
]]
rule("c++.meta.generators")


rule_end()