-- imports
import("core.base.option")
import("core.base.task")
import("core.base.global")
import("core.project.config")
import("core.project.project")
import("core.project.depend")
import("core.platform.platform")

local analyzing = analyzing or {}

function analyzing.filter_target(target_name)
    local attributes = target_attributes[target_name]
    if attributes and table.contains(attributes, "Analyze.Ignore") then
        return false
    end
    return true
end

function analyzing.load_targets()
    -- load config
    config.load()
    -- load targets
    project.load_targets()
end

target_attributes = {}
function analyzing.scan_attributes()
    for _, target in pairs(project.ordertargets()) do
        local attributes = target:values("Sakura.Attributes")
        if type(attributes) ~= 'table' then
            attributes = { attributes }
        end
        target_attributes[target:name()] = attributes or {}
    end
end

function analyzing.query_attributes(target_name)
    return target_attributes[target_name] or {}
end

analyzers = {}
target_infos = {}
function analyzing.run_analyzers()
    local phase = project.target("Analyze.Phase")
    local orderules = phase:orderules()
    local idx = 1
    for _, rule in ipairs(orderules) do
        if rule:name():startswith("__Analyzer.") then
            analyzers[idx] = rule
            idx = idx + 1
        end
    end
    for _, analyzer in ipairs(analyzers) do
        local analyzer_name = analyzer:name():split("__Analyzer.")[1]
        local analyze = analyzer:script("build")
        for _, target in pairs(project.ordertargets()) do
            local target_name = target:name()
            if analyzing.filter_target(target_name) then
                local attributes = target_attributes[target_name]
                local artifact = analyze(target, attributes, analyzing)
                target_infos[target_name] = target_infos[target_name] or {}
                target_infos[target_name][analyzer_name] = artifact
            end
        end
    end
end

function main()
    -- print("start analyze...")
    
    analyzing.load_targets()
    analyzing.scan_attributes()
    analyzing.run_analyzers()

    for target_name, info in pairs(target_infos) do
        local config_file = path.join("build", ".gens", "module_infos", target_name)
        io.save(config_file..".table", target_infos[target_name])
    end

    -- print("analyze ok!")
end