task("run-codegen-jobs")
    on_run(function (targetname)
        import("meta_codegen")
        meta_codegen(target)
    end)

rule("c++.codegen")
    on_load(function (target, opt)
        local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen")
        local target_gendir = path.join(gendir, target:name())

        if (not os.exists(target_gendir)) then
            os.mkdir(target_gendir)
        end

        target:data_set("meta.codegen.dir", gendir)
        target:add("includedirs", gendir, {public = true})
    end)
    on_config(function(target)
        local gendir = target:data("meta.codegen.dir")
        local sourcebatches = target:sourcebatches()
        if sourcebatches then
            local sourcebatch = sourcebatches["c++.build"]
            if sourcebatch then
                local sourcefiles = os.files(path.join(gendir, target:name(), "/generated.cpp"))
                for _, sourcefile in ipairs(sourcefiles) do
                    -- add source file to this batch
                    table.insert(sourcebatch.sourcefiles, sourcefile)
                    -- insert object files to source batches
                    local objectfile = target:objectfile(sourcefile, "cxx")
                    table.insert(sourcebatch.objectfiles, objectfile)
                    table.insert(sourcebatch.dependfiles, target:dependfile(objectfile))
                end
            end
        end
    end)