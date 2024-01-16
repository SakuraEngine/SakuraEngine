function daScript_AOTAndCompile(target, batchcmds, sourcefile_daS, opt)
    local binpath = target:targetdir()
    local cc = ""
    if is_plat("windows") then
        cc = path.join(binpath, "daScript.exe")
    else
        cc = path.join(binpath, "daScript")
    end
    if not os.exists(cc) then
        cc = "daScript"
    end
    local daC = { program = cc }
    -- get c/c++ source file for daScript
    local extension = path.extension(sourcefile_daS)
    local sourcefile_cx = path.join(target:autogendir(), "rules", "daS_aot", path.basename(sourcefile_daS) .. ".cpp")

    -- add objectfile
    local objectfile = target:objectfile(sourcefile_cx)
    table.insert(target:objectfiles(), objectfile)

    -- add commands
    batchcmds:show_progress(opt.progress, "${color.build.object}compiling.daS %s", sourcefile_daS)
    batchcmds:mkdir(path.directory(sourcefile_cx))
    batchcmds:vrunv(daC.program, {
        "-aot", path(sourcefile_daS), path(sourcefile_cx), 
        "-dasroot", path.join(os.scriptdir(), "..")
    })
    batchcmds:compile(sourcefile_cx, objectfile)

    -- add deps
    batchcmds:add_depfiles(sourcefile_daS)
    batchcmds:set_depmtime(os.mtime(objectfile))
    batchcmds:set_depcache(target:dependfile(objectfile))
end

function daScript_InstallScriptTo(target, batchcmds, sourcefile_daS, opt)
    local outdir = opt.outdir
    local rootdir = opt.rootdir
    local abs_source = path.absolute(sourcefile_daS)
    local rel_out = path.join(target:targetdir(), outdir)
    if (rootdir ~= "" or rootdir ~= nil) then
        local rel_root = path.relative(path.directory(abs_source), rootdir)
        rel_out = path.join(rel_out, rel_root)
    end
    local abs_out = path.absolute(rel_out).."/"..path.filename(sourcefile_daS)
    batchcmds:show_progress(opt.progress, "${color.build.object}%s.daScript ${clear}%s", target:name(), sourcefile_daS)
    batchcmds:cp(abs_source, abs_out)
    batchcmds:add_depfiles(abs_source)
    batchcmds:set_depmtime(os.mtime(abs_out))
    batchcmds:set_depcache(target:dependfile(abs_out))
end