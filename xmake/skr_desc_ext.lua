---------------------------------- global ----------------------------------
function skr_global_target()
    target("Skr.Global")
end

function skr_global_target_end()
    target_end()
end

function skr_global_config()
    -- policy
    set_policy("build.ccache", false)
    set_policy("build.warning", true)
    set_policy("check.auto_ignore_flags", false)
    
    -- cxx options
    set_warnings("all")
    set_languages(get_config("cxx_version"), get_config("c_version"))
    add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")

    -- cxx defines
    if (is_os("windows")) then 
        add_defines("UNICODE", "NOMINMAX", "_WINDOWS")
        add_defines("_GAMING_DESKTOP")
        add_defines("_CRT_SECURE_NO_WARNINGS")
        add_defines("_ENABLE_EXTENDED_ALIGNED_STORAGE")
        add_defines("_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR") -- for preventing std::mutex crash when lock
        if (is_mode("release")) then
            set_runtimes("MD")
        elseif (is_mode("asan")) then
            add_defines("_DISABLE_VECTOR_ANNOTATION")
        else
            -- set_runtimes("MDd")
            set_runtimes("MD")
        end
    end
end 

---------------------------------- env ----------------------------------
_skr_env_table = _skr_env_table or {}
_skr_env_committed = false

-- set env
function skr_env_set(key, value)
    _skr_env_table[key] = value
end

-- add env (trait value as list)
function skr_env_add(key, value)
    -- collect old value
    local old_value = _skr_env_table[key]
    if not old_value then
        old_value = {}
    elseif type(old_value) ~= "table" then
        old_value = { old_value }
    end

    -- insert new value
    table.insert(old_value, value)

    -- setup value
    _skr_env_table[key] = old_value
end

-- get env
function skr_env_get(key)
    return _skr_env_table[key]
end

-- commit env into skr global target
function skr_env_commit()
    if _skr_env_committed then
        cprint("${red} call 'skr_env_commit' after committed${clear}")
        exit(-1)
    end

    skr_global_target()
    set_values("skr.env", table.wrap_lock(_skr_env_table),{
        force = true,
        expand = false,
    })
    skr_global_target_end()

    _skr_env_committed = true
end

-- is env committed
function skr_env_is_committed()
    return _skr_env_committed
end

---------------------------------- analyze ----------------------------------
-- TODO. 使用更简洁的表述
-- begin analyzer target, for add analyzer
function analyzer_target(name)
    skr_global_target()
        add_rules("__Analyzer."..name)
    skr_global_target_end()
    rule("__Analyzer."..name)
end

-- end analyzer target, for add analyzer
function analyzer_target_end()
    rule_end()
end

-- analyze function
function analyze(func, opt)
    on_build(func, opt)
end

-- add analyzer attribute for target, that will be used by analyzer_target
function analyzer_attribute(attribute)
    add_values("Sakura.Attributes", attribute)
end

-- ignore target when perform analyze
function analyzer_ignore()
    analyzer_attribute("Analyze.Ignore")
end

---------------------------------- codegen ----------------------------------
-- add codegen component for target
function codegen_component(owner, opt)
    local no_codegen = opt.no_codegen
    target(owner)
        add_rules("c++.codegen.load", {no_codegen = no_codegen or false})
        analyzer_attribute("Codegen.Owner")
        if not no_codegen then
            add_deps(owner..".Mako", { public = opt and opt.public or true })
        else
            add_deps(owner..".Meta", { public = opt and opt.public or true })
        end
        add_values("c++.codegen.api", opt.api or target:name():upper())
        set_values("c++.codegen.enable", true)
    target_end()

    if not no_codegen then
        target(owner..".Mako")
            set_group("01.modules/"..owner.."/codegen")
            set_kind("headeronly")
            add_rules("c++.codegen.mako")
            add_rules("sakura.derived_target", { owner_name = owner })
            analyzer_ignore()
            set_policy("build.fence", true)
            add_deps(owner..".Meta", { public = true })
            on_load(function (target)
                target:data_set("mako.owner", owner)

                -- add deps
                import("skr.analyze")
                local analyze_tbl = analyze.load(owner)
                if analyze_tbl then
                    local depends = analyze_tbl["Codegen.MakoDeps"]
                    for _, dep in ipairs(depends) do
                        -- print("add codegen dependency: %s -> %s", target:name(), dep)
                        target:add("deps", dep, { public = true })
                    end
                else
                    -- print("failed to load analyze table for %s", target:name())
                end
            end)
    end
    -- must be declared at the end of this helper function
    target(owner..".Meta")
        set_group("01.modules/"..owner.."/codegen")
        set_kind("headeronly")
        add_rules("c++.codegen.meta")
        add_rules("sakura.derived_target", { owner_name = owner })
        analyzer_ignore()
        set_policy("build.fence", true)
        on_load(function (target)
            target:data_set("c++.codegen.owner", owner)
            if opt and opt.rootdir then
                opt.rootdir = path.absolute(path.join(target:scriptdir(), opt.rootdir))
            end
            target:data_set("meta.rootdir", opt.rootdir)
        end)
end

-- add codegen generator for target, that will be used when perform codegen
function codegen_generator(opt)
    add_rules("c++.codegen.generators", opt)
end

---------------------------------- pch ----------------------------------
function pch_target(owner_name, pch_target_name)
    target(pch_target_name)
        set_group("01.modules/"..owner_name.."/components")
        set_policy("build.fence", true)
        -- temporarily close the exception for pch target
        set_exceptions("no-cxx")
        analyzer_attribute("PCH")
        analyzer_ignore()
end

function private_pch(owner_name)
    target(owner_name)
        add_deps(owner_name..".PrivatePCH", {inherit = false})
        analyzer_attribute("PrivatePCH.Owner")
    target_end()

    pch_target(owner_name, owner_name..".PrivatePCH")
        -- private pch generate pch file and inject it to owner
        set_kind("headeronly")
        analyzer_attribute("PrivatePCH")
        add_rules("sakura.pcxxheader", { buildtarget = owner_name, shared = false })
        add_rules("sakura.derived_target", { owner_name = owner_name })
end

function shared_pch(owner_name)
    target(owner_name)
        analyzer_attribute("SharedPCH.Owner")
    target_end()

    pch_target(owner_name, owner_name..".SharedPCH")
        -- shared pch generate pch/obj file and link them to others
        set_kind("object") 
        add_rules("sakura.pcxxheader", { buildtarget = owner_name..".SharedPCH", shared = true })
        add_deps(owner_name)
        analyzer_attribute("SharedPCH")
        add_rules("sakura.derived_target", { owner_name = owner_name })
end

---------------------------------- install ----------------------------------
-- add install rule
function skr_install_rule()
    add_rules("skr.install")
end

--   kind: "download"/"files"/"custom"，不同的 kind 有不同的参数
--   [kind = any]: 共有参数
--     plat: filter target platform
--     arch: filter target architecture
--     toolchain； filter target toolchain
--   [kind = “download"]:
--     name: package or files name
--     install_func: "tool"/"resources"/"sdk"/"file"/"custom"/nil, nil means use install.lua in package
--     debug: wants download debug sdk
--     out_dir: override output directory
--     custom_install: if [install_func] is "custom", this function will be called
--     after_install: function to run after install
--   [kind = "files"]:
--     name: install name, for solve depend
--     files: list of files
--     out_dir: output directory
--     root_dir: root directory, nli mean erase directory structure when copy
--   [kind = "custom"]:
--     name: install name, for solve depend
--     func: function to run
function skr_install(kind, opt)
    add_values("skr.install", table.wrap_lock({
        kind = kind,
        opt = opt,
    }), {
        force = true,
        expand = false,
    })
end

---------------------------------- module ----------------------------------
function shared_module(name, api, opt)
    target(name)
        set_group("01.modules/"..name)
        add_rules("PickSharedPCH")
        add_rules("sakura.dyn_module", { api = api })
        if has_config("shipping_one_archive") then
            set_kind("static")
        else
            set_kind("shared")
        end
        on_load(function (target, opt)
            if has_config("shipping_one_archive") then
                for _, dep in pairs(target:get("links")) do
                    target:add("links", dep, {public = true})
                end
            end
        end)
        opt = opt or {}
        if opt.exception and not opt.noexception then
            set_exceptions("cxx")
        else
            set_exceptions("no-cxx")
        end
end

function static_component(name, owner, opt)
    target(owner)
        add_deps(name, { public = opt and opt.public or true })
    target_end()
    
    target(name)
        set_kind("static")
        set_group("01.modules/"..owner.."/components")
        add_rules("sakura.component", { owner = owner })
end

function executable_module(name, api, opt)
    target(name)
        set_kind("binary")
        add_rules("PickSharedPCH")
        add_rules("sakura.dyn_module", { api = api })
        local opt = opt or {}
        if opt.exception and not opt.noexception then
            set_exceptions("cxx")
        else
            set_exceptions("no-cxx")
        end
end

function public_dependency(dep, setting)
    add_deps(dep, {public = true})
    add_values("sakura.module.public_dependencies", dep)
end

---------------------------------- vsc_dbg ----------------------------------
-- @args: string list
function skr_dbg_args(args)
    add_values("vsc_dbg.args", args)
end

-- @envs: table, format (string, string)
function skr_dbg_envs(envs)
    for k, v in pairs(envs) do
        add_values("vsc_dbg.env", k)
        set_values("vsc_dbg.env."..k, v)
    end
end

-- @command: command string
function skr_dbg_cmd_pre(command)
    add_values("vsc_dbg.cmd_prev", command)
end

-- @command: command string
function skr_dbg_cmd_post(command)
    add_values("vsc_dbg.cmd_post", command)
end

-- proxy target for add launch task
-- @build_func: function(target, out_configs)
--  use table.insert to add values to [out_configs], config format
--     cmd_name: str (required)
--     program: str (required)
--     label: str (default use cmd_name)
--     args: str list
--     envs: str: str table
--     pre_cmds: str list
--     post_cmds: str list
--     natvis_files: str list
function skr_dbg_proxy_target(name, build_func)
    target(name)
        set_kind("phony")
        set_values("vsc_dbg.proxy_func", true)
        on_build(build_func)
    target_end()
end

-- custom target for directly add launch task
function skr_dbg_custom_target(name, build_func)
    target(name)
        set_kind("phony")
        set_values("vsc_dbg.custom_func", true)
        on_build(build_func)
    target_end()
end

function skr_dbg_natvis_files(...)
    add_values("vsc_dbg.natvis_files", ...)
end

---------------------------------- cull & tagged disable ----------------------------------
function skr_cull(name)
    skr_env_add("culled_modules", name)
end
function skr_includes_with_cull(name, func)
    local culled = skr_env_get("culled_modules")
    skr_env_add("modules_with_cull", name)
    if (not culled) or (not table.contains(culled, name)) then
        func()
    end
end

---------------------------------- unity build ----------------------------------
function skr_unity_build()
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
end