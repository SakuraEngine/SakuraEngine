target("SkrMetaCodegenPolicy")
    set_kind("headeronly")
    set_group("00.utilities")
    -- dispatch codegen task
    before_build(function(target)
        -- TODO. dispatch
    end)

rule("skr.meta")
    on_load(function (target, opt)
        -- add dependency
        target:add("deps", "SkrMetaCodegenPolicy")

        -- config
        local codegen_dir = path.join(target:autogendir({root = true}), target:plat(), "codegen")
        local source_dir = path.join(gendir, target:name(), "/generated.cpp")

        -- check generated files
        if not os.exists(source_dir) then
            local genfile = io.open(sourcefile, "w")                
            genfile:print("static_assert(false, \"codegen of module "..target:name().." is not completed!\")")
            genfile:close()
        end

        -- target configure
        target:add("files", sourcefile, { unity_ignored = true })
        target:add("includedirs", gendir, {public = true})
    end)
rule_end()

rule("skr.meta.generator")

rule_end()