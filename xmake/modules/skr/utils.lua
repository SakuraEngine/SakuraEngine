import("lib.detect")
import("core.project.config")
import("core.base.object")
import("core.base.option")
import("core.project.project")

------------------------dirs------------------------
function skr_build_artifact_dir()
    return "build/.skr/"
end
function skr_codegen_dir(target_name)
    return path.join(skr_build_artifact_dir(), "codegen", target_name)
end
function saved_config_path()
    return path.join(skr_build_artifact_dir(), "project.config")
end
function binary_dir()
    return vformat("$(builddir)/$(os)/$(arch)/$(mode)")
end
function download_dir()
    return path.join(skr_build_artifact_dir(), "download")
end
function install_dir_tool()
    -- TODO. no setup.lua
    -- return path.join(skr_build_artifact_dir(), vformat("tool/$(host)/$(arch)"))
    return path.join(skr_build_artifact_dir(), vformat("tool/$(host)"))
end
function install_dir_sdk(target)
    return target:targetdir()
end
function install_temp_dir(package_path)
    local base_name = path.basename(package_path)
    return path.join(skr_build_artifact_dir(), "install", base_name)
end
function depend_file(sub_path)
    return path.join(skr_build_artifact_dir(), "deps", sub_path..".d")
end
function depend_file_install(sub_path)
    return path.join(skr_build_artifact_dir(), vformat("deps/install/$(os)/$(arch)/$(mode)"), sub_path..".d")
end
function depend_file_target(target_name, sub_path)
    return path.join(
        skr_build_artifact_dir(), "deps", "target", target_name,
        vformat("$(os)/$(arch)/$(mode)"), sub_path..".d")
end
function flag_file(sub_path)
    return path.join(skr_build_artifact_dir(), "flags", sub_path..".flag")
end
function log_file(sub_path)
    return path.join(skr_build_artifact_dir(), "log", sub_path..".log")
end

------------------------config------------------------
function load_config()
    if not os.isfile(saved_config_path()) then
        raise("config file not found: %s", saved_config_path())
    end
    config.load(saved_config_path(), {force=true})
end
function save_config()
    config.save(saved_config_path(), {public=true})
end

------------------------env------------------------
function get_env(name)
    local global_target = project.target("Skr.Global")
    local env_value = global_target:values("skr.env")
    if not env_value then
        raise("lost global env table, did you forget call skr_env_commit()？")
    end
    return env_value[name]
end

------------------------package name rule------------------------
function package_name_tool(tool_name)
    return vformat("%s-$(host)-%s.zip", tool_name, os.arch())
end
function package_name_sdk(sdk_name, opt)
    opt = opt or {}

    -- TODO. $(host) -> $(os)
    if opt.debug then
        return vformat("%s_d-$(host)-%s.zip", sdk_name, os.arch())
    else
        return vformat("%s-$(host)-%s.zip", sdk_name, os.arch())
    end
end

------------------------find------------------------
function find_program_in_paths(program_name, paths, opt)
    opt = opt or {}

    -- bad case
    if path.is_absolute(program_name) then
        raise("program name cannot be an absolute path")
    end

    -- find program
    for _, _path in ipairs(paths) do
        -- format path
        _path = path.join(vformat(_path), program_name)

        -- check path
        if os.isexec(_path) then
            return _path
        end
    end
end
function find_program_in_system(program_name, opt)
    opt = opt or {}

    -- load opt
    local use_which = opt.use_sys_find or true;
    local use_env_find = opt.use_env_find or true;

    -- helpers
    local _format_program_name_for_winos = function (name)
        local name = program_name:lower()
        if not name:endswith(".exe") then
            name = name .. ".exe"
        end
        return name
    end
    local _check_program_path = function (path)
        if path then
            path = path:trim()
            if os.isexec(path) then
                return path
            end
        end
    end

    -- find use sys find
    if use_which then
        if os.host() == "windows" then -- use where
            -- format program name    
            local find_name = _format_program_name_for_winos(program_name)
            
            -- find
            local found_paths = nil
            try {
                function ()
                    found_paths = os.iorunv("where.exe", {find_name})
                end
            }
            if found_paths then
                for _, path in ipairs(found_paths:split("\n")) do
                    path = path:trim()
                    if #path > 0 then
                        if os.isexec(path) then
                            return path
                        end
                    end
                end
            end

        else -- use which
            local program_path = nil
            try {
                function ()
                    program_path = os.iorunv("which", {program_name})
                end
            }
            program_path = _check_program_path(program_path)
            if program_path then
                return program_path
            end
        end
    end

    -- find env find
    if use_env_find then
        if os.host() == "windows" then -- use registry
            -- local find_name = _format_program_name_for_winos(program_name)
            -- local program_path = winos.registry_query("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\" .. find_name)
            -- program_path = _check_program_path(program_path)
            -- if program_path then
            --     return program_path
            -- end
        else -- use sys default path
            local paths = {
                "/usr/local/bin",
                "/usr/bin",
            }
            local path = find_program_in_paths(program_name, paths, opt)
            if path then
                return path
            end
        end
    end
end
function find_download_file(file_name)
    return detect.find_file(file_name, download_dir())
end
function find_download_package(package_name)
    return find_download_file(package_name_sdk(package_name, opt))
end
function find_download_package_tool(tool_name)
    return find_download_file(package_name_tool(tool_name))
end
function find_download_package_sdk(sdk_name, opt)
    return find_download_file(package_name_sdk(sdk_name, opt))
end
function find_tool(tool_name, sub_paths, opt)
    -- combine paths
    local paths = { install_dir_tool() }
    for _, sub_path in ipairs(sub_paths) do
        table.insert(paths, path.join(install_dir_tool(), sub_path))
    end

    -- find
    return find_program_in_paths(tool_name, paths, opt)
end

------------------------find spec program------------------------
function find_python()
    -- find embedded_python
    local embedded_python = find_tool("python", {"python-embed"})
    if embedded_python then
        return embedded_python
    end

    -- find system python
    local python_name = (os.host() == "windows") and "python" or "python3"
    local system_python = find_program_in_system(python_name)
    if system_python then
        return system_python
    end
end
function find_meta()
    return find_tool("meta")
end
function find_bun()
    -- find embedded bun
    local embedded_bun = find_tool("bun")
    if embedded_bun then
        return embedded_bun
    end

    -- find system bun
    local system_bun = find_program_in_system("bun")
    if system_bun then
        return system_bun
    end
end

------------------------tools------------------------
-- change info: content: {
--   files = {
--     file = changed file,
--     reason = "new"|"modify"|"deleted", 
--       "new" means add to cache_file
--       "modify" means mtime miss match or sha256 miss match
--       "deleted" means file is deleted
--   }
--   values = {
--     value = changed value,
--     reason = see file reason
--   }
-- }
local ChangeInfo = ChangeInfo or object {}
function ChangeInfo:dump()
    local reason_fmt = {
        new = "${green}[%s]${clear}",
        modify = "${cyan}[%s]${clear}",
        deleted = "${red}[%s]${clear}",
    }
    
    -- dump files
    print("files:")
    for _, file in ipairs(self.files) do
        local fmt = reason_fmt[file.reason]

        if fmt then
            cprint(fmt.." %s", file.reason, file.file)
        else
            raise("unknown change reason: %s", file.reason)
        end
    end

    -- dump values
    print("values:")
    for _, value in ipairs(self.values) do
        local fmt = reason_fmt[value.reason]

        if fmt then
            cprint(fmt.." %s", value.reason, value.value)
        else
            raise("unknown change reason: %s", value.reason)
        end
    end
end
function ChangeInfo:is_file_changed()
    return #self.files > 0
end
function ChangeInfo:is_value_changed()
    return #self.values > 0
end
function ChangeInfo:any_changed()
    return self:is_file_changed() or self:is_value_changed()
end

-- solve change info for files
-- @param func: function to call when any change detected
-- @param opt: options:
--   cache_file: cache_file used to save time_stamp
--   files: files to check change
--   values: values to check change
--   use_sha: use sha256 to check file contents, this is useful when file time_stamp is not reliable
--   force: force to call func
--   post_scan: call after [func] called, used to append file to check next time, these files often are generated in [func]
-- @return: 
--   [0] is any file changed
--   [1] change info: see above
function on_changed(func, opt)
    -- load opt
    local opt = opt or {}
    local cache_file = opt.cache_file
    local files = opt.files
    local values = opt.values and table.wrap(opt.values) or nil
    local use_sha = opt.use_sha or false
    local force = opt.force or false
    local post_scan = opt.post_scan

    -- check opt
    if not cache_file then
        raise("cache_file is required")
    end
    
    -- load cache file
    local cache
    if not os.exists(cache_file) then
        cache = {
            files = {},
            values = {},
            post_files = {},
        }
    else
        cache = io.load(cache_file)
    end

    -- change info for cache changed
    local change_info = ChangeInfo {
        files = {},
        values = {},
        force = force,
    }

    -- check files
    if files then
        local checked_files = {}
        
        -- check input and cache diff
        for _, file in ipairs(files) do
            local cache_info = cache.files[file]
            
            -- solve mtime & sha
            local mtime, sha
            if os.exists(file) then
                mtime = os.mtime(file)
                sha = use_sha and hash.sha256(file) or ""
            else
                mtime = 0
                sha = ""
            end

            if not cache_info then -- new case
                -- add cache
                cache.files[file] = {
                    mtime = mtime,
                    sha256 = sha,
                }

                -- add change info
                table.insert(change_info.files, {
                    file = file,
                    reason = "new",
                })
            else -- modify case
                -- check modified
                local changed
                if use_sha then
                    changed = cache_info.sha256 ~= sha
                else
                    changed = cache_info.mtime ~= mtime
                end

                -- add change info
                if changed then
                    table.insert(change_info.files, {
                        file = file,
                        reason = "modify",
                    })
                end

                -- update cache
                cache.files[file] = {
                    mtime = mtime,
                    sha256 = sha,
                }
            end

            -- add to checked files
            checked_files[file] = true
        end

        -- check deleted files
        for file, _ in pairs(cache.files) do
            if not checked_files[file] then
                -- add change info
                table.insert(change_info.files, {
                    file = file,
                    reason = "deleted",
                })

                -- remove from cache
                cache.files[file] = nil
            end
        end
    end

    -- check values
    if values then
        -- check new value
        for value in ipairs(values) do
            if not table.contains(cache.values, value) then
                -- add change info
                table.insert(change_info.values, {
                    value = value,
                    reason = "new",
                })
            end
        end

        -- check deleted value
        for value in ipairs(cache.values) do
            if not table.contains(values, value) then
                -- add change info
                table.insert(change_info.values, {
                    value = value,
                    reason = "deleted",
                })
            end
        end

        -- update cache
        cache.values = values
    end

    -- check post file
    for file, cache_info in pairs(cache.post_files) do
        -- solve mtime & sha
        local mtime, sha
        if os.exists(file) then
            mtime = os.mtime(file)
            sha = use_sha and hash.sha256(file) or ""
        else
            mtime = 0
            sha = ""
        end

        -- check modified
        local changed
        if use_sha then
            changed = cache_info.sha256 ~= sha
        else
            changed = cache_info.mtime ~= mtime
        end

        -- add change info
        if changed then
            table.insert(change_info.files, {
                file = file,
                reason = "modify",
            })
        end
    end

    -- call func
    if change_info:any_changed() or force then
        func(change_info)

        -- do post scan
        if post_scan then
            local post_files = post_scan(change_info)
            if post_files then
                cache.post_files = {}
                for _, file in ipairs(post_files) do
                    -- insert check info
                    cache.post_files[file] = {
                        mtime = os.mtime(file),
                        sha256 = use_sha and hash.sha256(file) or "",
                    }
                end
            end
        end
    end

    -- save cache
    io.save(cache_file, cache)

    return change_info:any_changed(), change_info
end
function write_flag_file(sub_path)
    local path = flag_file(sub_path)
    io.writefile(path, "FLAG FILE FOR TRIGGER [utils.is_changed]")
end

------------------------lua utils------------------------
function setfenv(fn, env)
    local i = 1
    while true do
        local name = debug.getupvalue(fn, i)
        if name == "_ENV" then
        debug.upvaluejoin(fn, i, (function()
            return env
        end), 1)
        break
        elseif not name then
        break
        end

        i = i + 1
    end

    return fn
end
function getfenv(fn)
    local i = 1
    while true do
        local name, val = debug.getupvalue(fn, i)
        if name == "_ENV" then
            return val
        elseif not name then
            break
        end
        i = i + 1
    end
end
function redirect_fenv(fn, reference_fn)
    setfenv(fn, getfenv(reference_fn))
end