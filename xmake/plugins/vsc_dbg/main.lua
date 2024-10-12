import("core.base.option")
import("core.base.task")
import("core.base.global")
import("core.project.config")
import("core.project.project")
import("core.project.depend")
import("core.platform.platform")
import("core.base.json")

function main()
    -- load config
    config.load("build/.gens/analyze.conf")
    -- load targets
    project.load_targets()
    
    -- get data
    local plat = config.get("plat")
    local arch = config.get("arch")
    local mode = config.get("mode")
    local build_dir = format("${workspaceFolder}/build/%s/%s/%s", plat, arch, mode)

    -- get options
    local opt_targets = option.get("targets")

    -- collect targets
    local exec_targets = {}
    local dynmaic_targets = {}
    do
        function _add_target(target)
            local target_kind = target:kind()
            if target_kind == "binary" then
                table.insert(exec_targets, target)
            elseif target_kind == "shared" then
                table.insert(dynmaic_targets, target)
            end
        end
        if opt_targets and #opt_targets > 0 then -- collect targets from command line
            for _, target_name in ipairs(opt_targets) do
                local target = project.target(target_name)
                if target then
                    _add_target(target)
                else
                    cprint("${yellow}target [%s] not found!${clear}", target_name)
                end
            end
        else -- gloabl collect targets
            for _, target in pairs(project.ordertargets()) do
                _add_target(target)
            end
        end
    end

    -- generate debug configurations for exec targets
    local configs = {}
    local tasks = {}
    for _, target in ipairs(exec_targets) do
        table.insert(configs, {
            name = "▶️"..target:name(),
            type = "cppvsdbg",
            request = "launch",
            program = format("%s/%s.exe", build_dir, target:name()),
            args = json.mark_as_array({}),
            stopAtEntry = false,
            cwd = build_dir,
            environment = json.mark_as_array({}),
            console = "externalTerminal",
            preLaunchTask = "build "..target:name(),
        })
        table.insert(configs, {
            name = "🔍"..target:name(),
            type = "cppvsdbg",
            request = "attach",
            program = format("%s/%s.exe", build_dir, target:name()),
            args = json.mark_as_array({}),
            stopAtEntry = false,
            cwd = build_dir,
            environment = json.mark_as_array({}),
            console = "externalTerminal",
            -- preLaunchTask = "build "..target:name(),
        })
        table.insert(tasks, {
            label = "build "..target:name(),
            type = "shell",
            command = "xmake",
            args = {"build", target:name()},
            options = {
                cwd = "${workspaceFolder}",
            },
            group = {
                kind = "build",
                isDefault = false
            }
        })
    end

    -- generate debug configurations for dynamic targets
    for _, target in ipairs(dynmaic_targets) do
        table.insert(configs, {
            name = "🔍"..target:name(),
            type = "cppvsdbg",
            request = "attach",
            program = format("%s/%s.dll", build_dir, target:name()),
            args = json.mark_as_array({}),
            stopAtEntry = false,
            cwd = build_dir,
            environment = json.mark_as_array({}),
            console = "externalTerminal",
            -- preLaunchTask = "build "..target:name(),
        })
    end

    -- save debug configurations
    json.savefile(".vscode/launch.json", {
        version = "0.2.0",
        configurations = configs
    })
    json.savefile(".vscode/tasks.json", {
        version = "2.0.0",
        tasks = tasks
    })
end