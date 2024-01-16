target("SkrCodegenPolicy")
    set_kind("headeronly")
    set_group("00.utilities")
    -- dispatch codegen task
    before_build(function(target)
        import("meta_codegen")
        meta_codegen()
    end)

rule("c++.codegen")
    on_load(function (target, opt)
        target:add("deps", "SkrCodegenPolicy")

        local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen")
        local sourcefile = path.join(gendir, target:name(), "/generated.cpp")
        if (not os.exists(sourcefile)) then
            local genfile = io.open(sourcefile, "w")                
            genfile:print("static_assert(false, \"codegen of module "..target:name().." is not completed!\")")
            genfile:close()
        end
        target:add("files", sourcefile, { unity_ignored = true })
        target:add("includedirs", gendir, {public = true})
    end)