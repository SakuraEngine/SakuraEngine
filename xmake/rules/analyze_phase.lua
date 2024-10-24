-- target to trigger analyze when configure phase
target("Analyze.Phase")
    set_kind("phony")
    set_policy("build.fence", true)
    on_load(function(phase)
        import("core.base.option")
        import("core.project.config")
        import("core.project.depend")
        import("core.project.project")

        -- get raw argv
        argv = xmake.argv()

        -- filter clean task
        if argv[1] == "c" or argv[1] == "clean" then
            return
        end

        -- filter run script
        if argv[1] == "l" or argv[1] == "lua" then
            return
        end

        -- filter analyze task
        if argv[1] == "analyze_project" then
            return
        end

        -- build depend files
        local deps = {"build/.gens/analyze_phase.flag"}
        for _, file in ipairs(project.allfiles()) do
            table.insert(deps, file)
        end

        -- write flag files
        if not os.exists("build/.gens/analyze_phase.flag") then
            io.writefile("build/.gens/analyze_phase.flag", "flag to trigger analyze_phase")
        end

        -- write analyze config
        config.save("build/.gens/analyze.conf", {public=true})

        -- dispatch analyze
        depend.on_changed(function ()
            print("[Analyze.Phase]: trigger analyze with arg: "..table.concat(argv, " "))
            
            -- record trigger log
            local log_file = "build/.gens/analyze_trigger.log"
            local log_file_content = os.exists(log_file) and io.readfile(log_file) or ""
            local append_log_content = "["..os.date("%Y-%m-%d %H:%M:%S").."]: ".."trigger analyze with arg: "..table.concat(argv, " ").."\n"
            io.writefile(log_file, log_file_content..append_log_content)

            -- run analyze
            local out, err = os.iorun("xmake analyze_project")
            
            if out and #out > 0 then
                print("===================[Analyze Output]===================")
                printf(out)
                print("===================[Analyze Output]===================")
            end
            if err and #err > 0 then
                print("===================[Analyze Error]===================")
                printf(err)
                print("===================[Analyze Error]===================")
            end
            
        end, {dependfile = phase:dependfile("ANALYZE_PHASE"), files = deps})
    end)
    on_clean(function(phase)
        -- trigger next analyze
        io.writefile("build/.gens/analyze_phase.flag", "flag to trigger analyze_phase")
    end)
target_end()

-- analyze tool functions
function analyzer_target(name)
    target("Analyze.Phase")
        add_rules("__Analyzer."..name)
        set_default(false)
    target_end()
    rule("__Analyzer."..name)
end
function analyze(func, opt)
    on_build(func, opt)
end
function analyzer_target_end()
    rule_end()
end
function analyzer_attribute(attribute)
    add_values("Sakura.Attributes", attribute)
end
function analyzer_ignore()
    analyzer_attribute("Analyze.Ignore")
end

-- solve dependencies
analyzer_target("Dependencies")
    analyze(function(target, attributes, analyzing)
        local dependencies = {}
        local idx = 1
        for __, dep in ipairs(target:orderdeps()) do
            local dep_name = dep:name()
            if analyzing.filter_target(dep_name) then
                dependencies[idx] = dep_name
                idx = idx + 1
            end
        end
        return dependencies
    end)
analyzer_target_end()

-- solve attributes
analyzer_target("Attributes")
    analyze(function(target, attributes, analyzing)
        return attributes
    end)
analyzer_target_end()