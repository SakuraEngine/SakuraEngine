task("run-codegen-jobs")
    on_run(function ()
        import("core.base.scheduler")
        import("meta_codegen")
        scheduler.co_start(meta_codegen)
    end)

rule("c++.codegen")
    -- add_deps("c++")
    set_sourcekinds("cxx")
    --[[
    on_config(function (target, opt)
        import("meta_codegen")
        meta_codegen()
    end)
    --]]
    before_buildcmd_files(function(target, batchcmds, sourcebatch, opt)
        -- avoid duplicate linking of object files
        sourcebatch.objectfiles = {}
    end)
    on_buildcmd_files(function(target, batchcmds, sourcebatch, opt)
        import("core.base.scheduler")
        scheduler.co_group_wait(target:name()..".cpp-codegen")
        
        local cpp_batch = target:data("meta.batch.cpp")
        for _, file in ipairs(cpp_batch) do
            local sourcefile_cx = path.absolute(file)
            -- add objectfile
            local objectfile = target:objectfile(file)

            table.insert(target:objectfiles(), objectfile)

            -- add commands
            batchcmds:show_progress(opt.progress, "${color.build.object}compiling.codegen %s", sourcefile_cx)
            batchcmds:mkdir(path.directory(sourcefile_cx))
            batchcmds:compile(sourcefile_cx, objectfile)

            -- add deps
            batchcmds:add_depfiles(sourcefile_cx)
            batchcmds:set_depmtime(os.mtime(objectfile))
            batchcmds:set_depcache(target:dependfile(objectfile))
        end
    end)