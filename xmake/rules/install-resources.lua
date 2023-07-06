rule("utils.install-resources")
    before_buildcmd_file(function (target, batchcmds, sourcefile, opt)
        local raw_ext = path.extension(sourcefile)
        local ext = raw_ext:gsub('%.', '_')
        local outdir = target:extraconf("rules", "utils.install-resources", "outdir") or "/../resources"
        local rootdir = target:extraconf("rules", "utils.install-resources", "rootdir") or ""
        local ext_outdir = target:extraconf("rules", "utils.install-resources", ext.."_outdir")
        if(ext_outdir ~= nil) then
            outdir = ext_outdir
        end

        local abs_source = path.absolute(sourcefile)
        local rel_out = path.join(target:targetdir(), outdir)
        if (rootdir ~= "" or rootdir ~= nil) then
            local rel_root = path.relative(path.directory(abs_source), rootdir)
            rel_out = path.join(rel_out, rel_root)
        end
        local abs_out = path.absolute(rel_out).."/"..path.filename(sourcefile)
        batchcmds:show_progress(opt.progress, "${green}[%s]: install.resource ${clear}%s", target:name(), sourcefile)
        batchcmds:cp(abs_source, abs_out)

        batchcmds:add_depfiles(abs_source)
        batchcmds:set_depmtime(os.mtime(abs_out))
        batchcmds:set_depcache(target:dependfile(abs_out))
    end)

rule("utils.install-libs")
    set_extensions(".zip")
    on_load(function (target)
        import("find_sdk")
        local libnames = target:extraconf("rules", "utils.install-libs", "libnames")
        for _, libname in pairs(libnames) do
            local zip = find_sdk.find_sdk_lib(libname)
            target:add("files", zip)
        end
    end)
    before_buildcmd_file(function (target, batchcmds, zipfile, opt)
        import("find_sdk")
        import("core.project.depend")
        import("utils.archive")

        -- unzip to objectdir
        local tmpdir = path.join(target:objectdir(), path.basename(zipfile))
        local dependfile = target:dependfile(zipfile)

        depend.on_changed(function()
            archive.extract(zipfile, tmpdir)
        end, {dependfile = dependfile, files = { zipfile }})
    
        -- copy all files with batchcmds
        local vfiles = path.join(tmpdir, "**")
        local files = os.files(vfiles)
        batchcmds:show_progress(opt.progress, 
            "${green}[%s]: install.lib ${clear}%s, %d files", 
            target:name(), zipfile, #files)

        local outfiles = {}
        for _, file in ipairs(files) do
            local outfile = path.join(target:targetdir(), path.filename(file))
            outfile = path.absolute(outfile)
            table.insert(outfiles, outfile)
        end
        batchcmds:cp(vfiles, target:targetdir())
        batchcmds:add_depfiles(outfiles)
        batchcmds:set_depcache(target:dependfile(zipfile..".d"))
        batchcmds:set_depmtime(os.time())
    end)