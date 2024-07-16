rule("utils.install_resources")
    before_buildcmd_file(function (target, batchcmds, sourcefile, opt)
        local raw_ext = path.extension(sourcefile)
        local ext = raw_ext:gsub('%.', '_')
        local outdir = target:extraconf("rules", "utils.install_resources", "outdir") or "/../resources"
        local rootdir = target:extraconf("rules", "utils.install_resources", "rootdir") or ""
        local ext_outdir = target:extraconf("rules", "utils.install_resources", ext.."_outdir")
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

rule("utils.install_libraries")
    before_build(function (target)
        import("find_sdk")
        import("core.project.depend")
        import("utils.archive")

        local libnames = target:extraconf("rules", "utils.install_libraries", "libnames")
        for _, libname in pairs(libnames) do
            local zip = find_sdk.find_sdk_lib(libname)
            target:data_add("lib_zips", zip)
        end

        local tardir = target:targetdir()
        if not os.isdir(tardir) then
            os.mkdir(tardir)
        end

        local lib_zips = target:data("lib_zips")
        if not lib_zips or #lib_zips == 0 then
            return
        end
        
        for _, zipfile in ipairs(lib_zips) do
            -- unzip to objectdir
            local tmpdir = path.join(target:objectdir(), path.basename(zipfile))
            local dependfile = target:dependfile(zipfile..".unzip")

            depend.on_changed(function()
                archive.extract(zipfile, tmpdir)
            end, {dependfile = dependfile, lastmtime = os.mtime(zipfile), files = { zipfile }})
        
            local vfiles = path.join(tmpdir, "**")
            local files = os.files(vfiles)
            local outfiles = {}
            for _, file in ipairs(files) do
                local outfile = path.join(target:targetdir(), path.filename(file))
                outfile = path.absolute(outfile)
                table.insert(outfiles, outfile)
            end

            local dependfile2 = target:dependfile(zipfile..".cp")
            depend.on_changed(function()
                cprint("${green}[%s]: install.lib ${clear} %s, %d files", target:name(), zipfile, #vfiles)
                os.cp(vfiles, target:targetdir())
            end, {dependfile = dependfile2, lastmtime = os.mtime(zipfile), files = outfiles})
        end
    end)