target("Analyze.Phase")
    set_kind("phony")
    set_policy("build.fence", true)
    on_load(function(target)
        import("core.project.project")
        if not project.__noscan then
            os.exec("xmake analyze_project")
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