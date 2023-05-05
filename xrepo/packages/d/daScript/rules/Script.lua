rule("Script")
    set_extensions(".das")
    on_buildcmd_file(function (target, batchcmds, sourcefile_daS, opt)
        local task = import("./../modules/daScriptTasks")
        opt.outdir = target:extraconf("rules", "@daScript/Script", "outdir") or "./"
        opt.rootdir = target:extraconf("rules", "@daScript/Script", "rootdir") or ""
        task.daScript_InstallScriptTo(target, batchcmds, sourcefile_daS, opt)
    end)