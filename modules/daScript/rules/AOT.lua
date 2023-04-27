target("daScriptAOTTemplate")
    set_group("01.modules")
    set_kind("static")
    set_optimize("fastest")
    add_rules("@daScript/AOT", { noinstall = true })
    add_packages("daScript", { public = true })
    add_files("./../none.c")

rule("daScript")
    add_deps("c++")
    set_extensions(".das")
    after_load(function (target)
        -- collect aot files
        local aot_files = {}
        for _, source in ipairs(target:sourcefiles()) do
            if path.extension(source) == ".das" then
                local fileconfig = target:fileconfig(source)
                if fileconfig and fileconfig.aot then
                    table.insert(aot_files, source)
                end
            end
        end
        if #aot_files == 0 then
            return
        end

        -- fork template target
        import("core.project.project")
        local template = project.target("daScriptAOTTemplate")
        local aot = template:clone()
        aot:name_set(target:name().."_dasAOT")
        for _, source in ipairs(aot_files) do
            table.insert(aot:sourcefiles(), source)
        end
        local wholearchive = function(target, aot)
            local output_dir = vformat("$(buildir)/$(os)/$(arch)/$(mode)")
            if is_plat("linux") then
                target:add("ldflags", "-Wl,--whole-archive "..output_dir.."/lib"..aot:name()..".a -Wl,--no-whole-archive", {force = true, public = false})
            elseif is_plat("macosx") then
                target:add("ldflags", "-Wl,-force_load "..output_dir.."/lib"..aot:name()..".a", {force = true, public = false})
            elseif is_plat("windows") then
                target:add("ldflags", "/WHOLEARCHIVE:"..aot:name()..".lib", {force = true, public = false})
            end
        end

        -- deps
        if target:kind() == "binary" then
            wholearchive(target, aot)
            target:add("deps", aot:name())
        else
            aot:add("deps", target:name())        
            for _, other in pairs(project.targets()) do
                if other:kind() == "binary" and other:dep(target:name()) ~= nil then 
                    wholearchive(other, aot)
                    other:add("deps", aot:name())
                end
            end
        end
        
        -- add to project
        project.target_add(aot)
    end)
    on_buildcmd_file(function (target, batchcmds, sourcefile_daS, opt)
        import("core.project.project")
        local task = import("./../modules/daScriptTasks")
        opt.outdir = target:extraconf("rules", "daScript", "outdir") or "./"
        opt.rootdir = target:extraconf("rules", "daScript", "rootdir") or ""
        task.daScript_InstallScriptTo(target, batchcmds, sourcefile_daS, opt)
    end)