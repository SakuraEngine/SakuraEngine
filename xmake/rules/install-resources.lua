rule("utils.install-resources")
    after_buildcmd_file(function (target, batchcmds, sourcefile, opt)
        local raw_ext = path.extension(sourcefile)
        local ext = raw_ext:gsub('%.', '_')
        local outdir = target:extraconf("rules", "utils.install-resources", "outdir") or "/../resources"
        local ext_outdir = target:extraconf("rules", "utils.install-resources", ext.."_outdir")
        if(ext_outdir ~= nil) then
            outdir = ext_outdir
        end
        os.cp(sourcefile, path.join(target:targetdir(), outdir).."/")
    end)