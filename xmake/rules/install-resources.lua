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
        batchcmds:show_progress(opt.progress, "${green}%s.install.resource ${clear}%s", target:name(), sourcefile)
        batchcmds:cp(abs_source, abs_out)

        batchcmds:add_depfiles(abs_source)
        batchcmds:set_depmtime(os.mtime(abs_out))
        batchcmds:set_depcache(target:dependfile(abs_out))
    end)