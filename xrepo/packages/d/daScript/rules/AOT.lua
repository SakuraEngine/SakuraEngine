rule("AOT")
    add_deps("c++")
    set_extensions(".das")
    on_buildcmd_file(function (target, batchcmds, sourcefile_daS, opt)
        local binpath = path.join(target:pkg("daScriptTool"):installdir(), "bin")
        local cc = ""
        if is_plat("windows") then
            cc = path.join(binpath, "daScript.exe")
        else
            cc = path.join(binpath, "daScript")
        end
        if not os.exists(cc) then
            cc = "daScript"
            print("...")
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
    end)

