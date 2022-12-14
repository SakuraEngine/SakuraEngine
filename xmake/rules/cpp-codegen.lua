task("run-codegen-jobs")
    on_run(function (targetname)
        import("meta_codegen")
        if(has_config("use_async_codegen")) then
            import("core.base.scheduler")
            scheduler.co_start(meta_codegen, targetname)
        else
            meta_codegen(target)
        end
    end)

rule("c++.codegen")
    -- add_deps("c++")
    set_sourcekinds("cxx")
    on_load(function (target, opt)
        local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen")
        local target_gendir = path.join(gendir, target:name())

        if (not os.exists(target_gendir)) then
            local nullcontent = "#ifndef __meta__\nstatic_assert(0, \"incomplete file included!\");\n#endif"
            io.writefile(path.join(target_gendir, "module.configure.h"), nullcontent)
        end

        target:data_set("meta.codegen.dir", gendir)
        target:add("includedirs", gendir, {public = true})
    end)

    before_build_files(function (target, sourcebatch, opt)
        -- avoid duplicate linking of object files
        sourcebatch.objectfiles = {}
        -- wait async codegen finish
        if(has_config("use_async_codegen")) then
            import("core.base.scheduler")
            scheduler.co_group_wait(target:name()..".cpp-codegen")
        end
    end)

    before_buildcmd_files(function(target, batchcmds, sourcebatch, opt)
        -- avoid duplicate linking of object files
        sourcebatch.objectfiles = {}
    end)

    on_buildcmd_files(function(target, batchcmds, sourcebatch, opt)
        -- add to sourcebatch
        local gendir = target:data("meta.codegen.dir")
        local sourcebatches = target:sourcebatches()
        local cppfiles = os.files(path.join(gendir, target:name(), "/*.cpp"))
        
        -- compile generated cpp files
        for _, file in ipairs(cppfiles) do
            local sourcefile_cx = path.absolute(file)
            -- add objectfile
            local objectfile = target:objectfile(file)

            table.insert(target:objectfiles(), objectfile)

            if not opt.quiet then
                batchcmds:show_progress(opt.progress, "${color.build.object}[%s]: compiling.codegen %s", target:name(), file)
            end

            -- add commands
            batchcmds:mkdir(path.directory(sourcefile_cx))
            batchcmds:compile(sourcefile_cx, objectfile, {configs = {includedirs = gendir, languages = "c++17"}})

            -- add deps
            batchcmds:add_depfiles(sourcefile_cx)
            batchcmds:set_depmtime(os.mtime(objectfile))
            batchcmds:set_depcache(target:dependfile(objectfile))
        end
    end)