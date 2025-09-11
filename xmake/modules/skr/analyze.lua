
import("core.base.option")
import("core.project.config")
import("core.project.project")
import("skr.utils")

-----------------------dir & file-----------------------
-- load saved data
function analyze_file_name(target_name)
    return path.join(utils.skr_build_artifact_dir(), "analyze", target_name .. ".table")
end

-----------------------attributes-----------------------
-- load attributes set by skr_attributes()
function load_attributes(target)
    local attributes = target:values("Sakura.Attributes")
    if type(attributes) ~= "table" then
        attributes = { attributes }
    end
    return attributes or {}
end

-- filter target to perform analyze
function filter_target(target, attributes)
    attributes = attributes or load_attributes(target)
    if attributes and attributes["Analyze.Ignore"] then
        return false
    end
    return true
end

-----------------------analyzers-----------------------
-- load analyzer functions
function load_analyzers()
    local analyzer_target = project.target("Skr.Global")
    
    -- collect analyzer rules
    local analyzer_rules = {}
    for _, rule in ipairs(analyzer_target:orderules()) do
        if rule:name():startswith("__Analyzer.") then
            table.insert(analyzer_rules, rule)
        end
    end

    -- load analyzer functions
    local analyzers = {}
    for _, analyzer in ipairs(analyzer_rules) do
        local analyzer_name = analyzer:name():split("__Analyzer.")[1]
        local analyze_func = analyzer:script("build")
        table.insert(analyzers, {
            name = analyzer_name, 
            func = analyze_func
        })
    end

    return analyzers
end

-----------------------analyzer archive-----------------------
_analyze_cache = _analyze_cache or {}
function load(target_name)
    -- load from cache
    local exist_analyze = _analyze_cache[target_name]
    if exist_analyze then
        return exist_analyze
    end

    -- load from file
    local path = analyze_file_name(target_name)
    if os.exists(path) then
        local loaded_analyze = io.load(path)
        if loaded_analyze then
            _analyze_cache[target_name] = loaded_analyze
            return loaded_analyze
        end
    end
    return nil
end
function save(target_name, table)
    -- save to cache
    _analyze_cache[target_name] = table

    -- save analyze
    io.save(analyze_file_name(target_name), table)
end
function clear_cache()
    _analyze_cache = {}
end
function clear_cache_for(target_name)
    _analyze_cache[target_name] = nil
end

-----------------------trigger analyze tools-----------------------
function filter_analyze_trigger()
    -- get raw argv
    argv = xmake.argv()

    -- filter clean task
    if argv[1] == "c" or argv[1] == "clean" then
        return false
    end

    -- filter run script
    if argv[1] == "l" or argv[1] == "lua" then
        return false
    end

    -- filter analyze task
    if argv[1] == "analyze_project" then
        return false
    end

    return true
end
function trigger_analyze()
    -- build depend files
    local build_flag_path = utils.flag_file("ANALYZE_PHASE")
    local deps = {build_flag_path}
    for _, file in ipairs(project.allfiles()) do
        table.insert(deps, file)
    end

    -- write flag files
    if not os.exists(build_flag_path) then
        utils.write_flag_file("ANALYZE_PHASE")
    end

    -- dispatch analyze
    utils.on_changed(function(change_info)
        import("core.base.task")
        -- record trigger log
        local flag_file = utils.log_file("_analyze_flag")
        local log_file = utils.log_file("analyze_trigger")
        local magic_code = "114514_1919810"
        if not os.exists(flag_file) or io.readfile(flag_file) ~= magic_code then
            cprint("${cyan}[ANALYZE]: trigger analyze with arg: %s${clear}", table.concat(argv, " "))
            local log_file_content = os.exists(log_file) and io.readfile(log_file) or ""
            local append_log_content = "[" .. os.date("%Y-%m-%d %H:%M:%S") .. "]: " .. "trigger analyze with arg: " ..
                                           table.concat(argv, " ") .. "\n"
            io.writefile(log_file, log_file_content .. append_log_content)
            io.writefile(flag_file, magic_code)
            -- run analyze
            -- local out, err = os.iorun("xmake analyze_project")

            -- if out and #out > 0 then
            --     print("===================[Analyze Output]===================")
            --     printf(out)
            --     print("===================[Analyze Output]===================")
            -- end
            -- if err and #err > 0 then
            --     print("===================[Analyze Error]===================")
            --     printf(err)
            --     print("===================[Analyze Error]===================")
            -- end
            task.run("analyze_project")
        else
            os.rm(flag_file)
        end
    end, {
        cache_file = utils.depend_file("ANALYZE_PHASE"),
        files = deps,
    })
end
function write_analyze_trigger_flag()
    utils.write_flag_file("ANALYZE_PHASE")
end
function clean_analyze_files()
    os.rm(path.join(utils.skr_build_artifact_dir(), "analyze"))
end
function in_analyze_phase()
    return xmake.argv()[1] == "analyze_project"
end