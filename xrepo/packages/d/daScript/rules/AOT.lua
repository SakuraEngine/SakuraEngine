rule("AOT")
    add_deps("c++")
    set_extensions(".das")
    on_buildcmd_file(function (target, batchcmds, sourcefile_daS, opt)
        local task = import("./../modules/daScriptTasks")
        opt.outdir = target:extraconf("rules", "@daScript/AOT", "outdir") or "./"
        opt.rootdir = target:extraconf("rules", "@daScript/AOT", "rootdir") or ""
        if not opt.noinstall then
            task.daScript_InstallScriptTo(target, batchcmds, sourcefile_daS, opt)
        end
        task.daScript_AOTAndCompile(target, batchcmds, sourcefile_daS, opt)
    end)