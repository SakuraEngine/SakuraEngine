task("run-codegen-jobs")
    on_run(function (targetname)
        import("core.base.scheduler")
        import("meta_codegen")
        scheduler.co_start(meta_codegen, targetname)
    end)

rule("c++.codegen")
    -- add_deps("c++")
    set_sourcekinds("cxx")
    on_config(function (target, opt)
        local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen")

        if (not os.exists(gendir)) then
            io.open(path.join(gendir, "dummy.json"), "w")
        end

        target:data_set("meta.codegen.dir", gendir)
        target:add("includedirs", gendir, {public = true})
        target:add("includedirs", private_gendir)
    end)
    
    before_build(function(target)
        import("core.base.scheduler")
        scheduler.co_group_wait(target:name()..".cpp-codegen.early_mako")
        scheduler.co_group_wait(target:name()..".cpp-codegen.meta")
        scheduler.co_group_wait(target:name()..".cpp-codegen.weak_mako")
        scheduler.co_group_wait(target:name()..".cpp-codegen.strong_mako")
    end)

    before_buildcmd_files(function(target, batchcmds, sourcebatch, opt)
        -- avoid duplicate linking of object files
        sourcebatch.objectfiles = {}
    end)

    on_buildcmd_files(function(target, batchcmds, sourcebatch, opt)
        -- add to sourcebatch
        local gendir = target:data("meta.codegen.dir")
        local sourcebatches = target:sourcebatches()
        local cppfiles = os.files(path.join(gendir, "/**.cpp"))
        
        -- compile generated cpp files
        for _, file in ipairs(cppfiles) do
            local sourcefile_cx = path.absolute(file)
            -- add objectfile
            local objectfile = target:objectfile(file)

            table.insert(target:objectfiles(), objectfile)

            if not opt.quiet then
                batchcmds:show_progress(opt.progress, "${color.build.object}compiling.codegen %s", file)
            end

            -- add commands
            batchcmds:mkdir(path.directory(sourcefile_cx))
            batchcmds:compile(sourcefile_cx, objectfile)

            -- add deps
            batchcmds:add_depfiles(sourcefile_cx)
            batchcmds:set_depmtime(os.mtime(objectfile))
            batchcmds:set_depcache(target:dependfile(objectfile))
        end
    end)