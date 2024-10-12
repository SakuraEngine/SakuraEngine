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

    -- scan exec targets
    local exec_targets = {}
    for _, target in pairs(project.ordertargets()) do
        if target:kind() == "binary" then
            local target_name = target:name()
            local target_group = target:get("group")
            table.insert(exec_targets, target)
        end
    end

    -- generate debug configurations
    local configs = {}
    local tasks = {}
    for _, target in ipairs(exec_targets) do
        table.insert(configs, {
            name = "[Launch] "..target:name(),
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