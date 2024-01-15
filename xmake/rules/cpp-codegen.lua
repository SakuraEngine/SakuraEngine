task("run-codegen-jobs")
    on_run(function (targetname)
        import("meta_codegen")
        meta_codegen(target)
    end)

rule("c++.codegen")
    on_load(function (target, opt)
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