target("Analyze.Phase")
    set_kind("phony")
    set_policy("build.fence", true)
    on_load(function(phase)
        import("core.project.depend")
        import("core.project.project")

        local need_analyze = false
        if not project.__noscan then
            depend.on_changed(function ()
                need_analyze = true
                os.run("xmake analyze_project")
            end, {dependfile = phase:dependfile("ANALYZE_PHASE"), files = project.allfiles()})
        end
        
        if not need_analyze then
            print("no need to analyze project!")
        end
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

analyzer("Attributes")
    analyze(function(target, attributes, analyzing)
        return attributes
    end)