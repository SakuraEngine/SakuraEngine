target("Analyze.Phase")
    set_kind("phony")
    set_policy("build.fence", true)
    on_load(function(phase)
        import("core.base.option")
        import("core.project.depend")
        import("core.project.project")

        -- get raw argv
        argv = xmake.argv()

        -- filter clean task
        if argv[1] == "c" or argv[1] == "clean" then
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
        io.writefile("build/.gens/analyze_phase.flag", "flag to trigger analyze_phase")

        -- dispatch analyze
        depend.on_changed(function ()
            print("[Analyze.Phase]: trigger analyze")
            local out, err = os.iorun("xmake analyze_project")
            
            -- if out and #out > 0 then
            --     print(out)
            -- end
            -- if err and #err > 0 then
            --     print(err)
            -- end
            
        end, {dependfile = phase:dependfile("ANALYZE_PHASE"), files = deps})
    end)
    on_clean(function(phase)
        -- trigger next analyze
        io.writefile("build/.gens/analyze_phase.flag", "flag to trigger analyze_phase")
    end)
target_end()

function analyzer(name)
    target("Analyze.Phase")
        add_rules("__Analyzer."..name)
    target_end()
    rule("__Analyzer."..name)
end

function analyzer_end()
    rule_end()
end

function analyze(func, opt)
    on_build(func, opt)
end

function add_attribute(attribute)
    add_values("Sakura.Attributes", attribute)
end

analyzer("Dependencies")
    analyze(function(target, attributes, analyzing)
        local dependencies = {}
        for __, dep in pairs(target:orderdeps()) do
            local dep_name = dep:name()
            if analyzing.filter_target(dep_name) then
                table.insert(dependencies, dep_name)
            end
        end
        return dependencies
    end)
analyzer_end()

analyzer("Attributes")
    analyze(function(target, attributes, analyzing)
        return attributes
    end)
analyzer_end()