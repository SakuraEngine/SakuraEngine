rule("Hybrid")
    add_deps("c++")
    set_extensions(".das")
    on_buildcmd_file(function (target, batchcmds, sourcefile_daS, opt)
        local task = import("./../modules/daScriptTasks")
        opt.outdir = target:extraconf("rules", "@daScript/Hybrid", "outdir") or "./"
        opt.rootdir = target:extraconf("rules", "@daScript/Hybrid", "rootdir") or ""
        local fileconfig = target:fileconfig(sourcefile_daS)
        if fileconfig and fileconfig.aot then
            task.daScript_AOTAndCompile(target, batchcmds, sourcefile_daS, opt)
        end
        task.daScript_InstallScriptTo(target, batchcmds, sourcefile_daS, opt)
    end)