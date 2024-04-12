import("core.base.option")
import("core.base.scheduler")
import("core.project.depend")
import("core.project.project")
import("core.language.language")
import("core.tool.compiler")
import("find_sdk")
import("core.base.json")

-- programs
local _meta = find_sdk.find_program("meta")
local _python = find_sdk.find_embed_python() or find_sdk.find_program("python3")

-- data cache names
local _meta_data_batch_name = "meta.batch"
local _meta_data_headers_name = "meta.headers"
local _meta_data_generators_name = "meta.generators"

-- rule names
local _meta_rule_codegen_name = "c++.codegen" -- TODO. use new name
local _meta_rule_generators_name = "c++.meta.generators"

-- commands
function _compile_task(compile_func, target, opt)
    -- get config
    local refl_batch = target:data(_meta_data_batch_name ) or {}
    local rootdir = target:extraconf("rules", _meta_rule_codegen_name, "rootdir")
    rootdir = path.absolute(path.join(target:scriptdir(), rootdir))
    
    -- call compile function
    for _, sourceinfo in ipairs(refl_batch) do
        sourceinfo = sourceinfo or {}
        local headerfiles = sourceinfo.headerfiles
        if headerfiles then
            compile_func(
                target
                , rootdir
                , sourceinfo.metadir
                , sourceinfo.gendir
                , sourceinfo.sourcefile
                , headerfiles
                , opt
            )
        end
    end
end
function _meta_compile_command(sourcefile, rootdir, outdir, target, opt)
    opt = opt or {}
    opt.target = target
    opt.rawargs = true
    local start_time = os.time()
    local using_msvc = target:toolchain("msvc")
    local using_clang_cl = target:toolchain("clang-cl")

    -- load compiler and get compilation command
    local sourcekind = opt.sourcekind
    if not sourcekind and type(sourcefile) == "string" then
        sourcekind = language.sourcekind_of(sourcefile)
    end
    local compiler_inst = compiler.load(sourcekind, opt)
    local program, argv = compiler_inst:compargv(sourcefile, sourcefile..".o", opt)
    if using_msvc or using_clang_cl then
        table.insert(argv, "--driver-mode=cl")
        --table.insert(argv, "/Tp")
    else
        --table.insert(argv, "-x c++")
    end
    table.insert(argv, "-I"..os.projectdir()..vformat("/SDKs/tools/$(host)/meta-include"))

    -- hack: insert a placeholder to avoid the case where (#argv < limit) and (#argv + #argv2 > limit)
    local argv2 = {
        sourcefile, 
        "--output="..path.absolute(outdir), 
        "--root="..rootdir or path.absolute(target:scriptdir()), 
        "--"
    }
    if is_host("windows") then
        local argv2_len = 0
        for _, v in pairs(argv2) do
            argv2_len = argv2_len + #v
        end
        local hack_opt = "-DFUCK_YOU_WINDOWS" .. string.rep("S", argv2_len)
        table.insert(argv, #argv + 1, hack_opt)
        argv = winos.cmdargv(argv)
    end
    for k,v in pairs(argv2) do  
        table.insert(argv, k, v)
    end
    
    -- print commands
    local command = program .. " " .. table.concat(argv, " ")
    if not opt.quiet then
        cprint(
            "${green}[%s]: compiling.meta ${clear}%ss"
            , target:name()
            , path.relative(outdir)
            -- , command
        )
    end

    -- compile meta source
    os.runv(_meta.program, argv)
    if not opt.quiet then
        cprint(
            "${green}[%s]: finish.meta ${clear}%s cost ${red}%d seconds"
            , target:name()
            , path.relative(outdir)
            , os.time() - start_time
        )
    end
end
function _meta_codegen_command_old(target, scripts, metadir, gendir, opt)
    -- get config
    local api = target:extraconf("rules", _meta_rule_codegen_name, "api")
    local generator = os.projectdir()..vformat("/tools/meta_codegen/_deprecated/codegen.py")
    local start_time = os.time()
    
    -- collect deps data
    local meta_deps_dir = {}
    for _, dep in pairs(target:deps()) do
        local depmetadir = path.join(dep:autogendir({root = true}), dep:plat(), "reflection/meta")
        table.insert(meta_deps_dir, depmetadir)
    end

    if not opt.quiet then
        cprint("${cyan}[%s]: %s${clear} %s", target:name(), path.filename(generator), path.relative(metadir))
    end
    
    -- baisc commands
    local command = {
        generator,
        "-root", path.absolute(metadir),
        "-outdir", gendir,
        "-api", api and api:upper() or target:name():upper(),
        "-module", target:name(),
    }

    -- strong order
    table.insert(command, "-includes")
    for _, dep in ipairs(meta_deps_dir) do
        table.insert(command, dep)
    end

    -- generators
    table.insert(command, "-generators")
    for _, script in pairs(scripts) do
        table.insert(command, script.file)
    end

    if verbos then
        cprint(
            "[%s] python %s"
            , target:name()
            , table.concat(command, " ")
        )
    end

    -- call codegen script
    os.iorunv(_python.program, command)

    if not opt.quiet then
        cprint(
            "${cyan}[%s]: %s${clear} %s cost ${red}%d seconds"
            , target:name()
            , path.filename(generator)
            , path.relative(metadir)
            , os.time() - start_time)
    end
end
function _meta_codegen_command(target, scripts, metadir, gendir, opt)
    -- get config
    local api = target:extraconf("rules", _meta_rule_codegen_name, "api")
    local generate_script = os.projectdir()..vformat("/tools/meta_codegen/codegen.py")
    local start_time = os.time()

    if not opt.quiet then
        cprint("${cyan}[%s]: %s${clear} %s", target:name(), path.filename(generate_script), path.relative(metadir))
    end

    -- config
    local config = {
        output_dir = gendir,
        main_module = {
            module_name = target:name(),
            meta_dir = path.absolute(metadir),
            api = api and api:upper() or target:name():upper(),
        },
    }
    

    -- collect include modules
    config.include_modules = {}
    for _, dep_target in pairs(target:deps()) do
        local dep_api = dep_target:extraconf("rules", _meta_rule_codegen_name, "api")

        table.insert(config.include_modules, {
            module_name = dep_target:name(),
            meta_dir = path.join(dep_target:autogendir({root = true}), dep_target:plat(), "reflection/meta"),
            api = dep_api and dep_api:upper() or dep_target:name():upper(),
        })
    end

    -- collect generators
    config.generators = {}
    for _, script in pairs(scripts) do
        table.insert(config.generators, {
            entry_file = script.file,
            import_dirs = script.import_dirs,
            use_new_framework = script.use_new_framework,
        })
    end

    -- output config
    config_file = path.join(target:autogendir({root = true}), target:plat(), "codegen/meta_codegen_config.json")
    json.savefile(config_file, config)

    -- baisc commands
    local command = {
        generate_script,
        "--config", config_file
    }

    if verbos then
        cprint(
            "[%s] python %s"
            , target:name()
            , table.concat(command, " ")
        )
    end

    -- call codegen script
    local result = os.iorunv(_python.program, command)
    -- os.execv(_python.program, command)
    
    if result and #result > 0 then
        print(result)
    end

    if not opt.quiet then
        cprint(
            "${cyan}[%s]: %s${clear} %s cost ${red}%d seconds"
            , target:name()
            , path.filename(generate_script)
            , path.relative(metadir)
            , os.time() - start_time)
    end
end

-- steps
--  1. collect header batch
--  2. solve generators
--  3. compile headers to meta json
--  4. call codegen scripts
function _collect_headers_batch(target)
    -- get config
    local headerfiles = {}
    local files = target:extraconf("rules", _meta_rule_codegen_name, "files")
    for _, file in ipairs(files) do
        local match_files = os.files(path.join(target:scriptdir(), file))
        for __, filepath in ipairs(match_files) do
            table.insert(headerfiles, filepath)
        end
    end
    local batchsize = 999 -- batch size optimization is disabled now
    local sourcedir = path.join(target:autogendir({root = true}), target:plat(), "reflection/src")
    local metadir = path.join(target:autogendir({root = true}), target:plat(), "reflection/meta")
    local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen", target:name())

    -- build batches
    local meta_batch = {}
    local batch_id = 1
    local file_count = 0
    for _, headerfile in pairs(headerfiles) do
        -- new batch
        if batchsize and file_count >= batchsize then
            batch_id = batch_id + 1
            file_count = 0
        end

        -- add source to batch
        local sourcefile = path.join(sourcedir, "reflection_" .. tostring(batch_id) .. ".cpp")
        if sourcefile then
            local sourceinfo = meta_batch[batch_id]
            if not sourceinfo then
                sourceinfo = {
                    sourcefile = sourcefile,
                    metadir = metadir,
                    gendir = gendir,
                    headerfiles = {}
                }
                meta_batch[batch_id] = sourceinfo
            end
            table.insert(sourceinfo.headerfiles, headerfile)
        end

        file_count = file_count + 1
    end

    -- set data
    target:data_set(_meta_data_batch_name, meta_batch)
    target:data_set(_meta_data_headers_name, headerfiles)
end
function _solve_generators(target)
    -- get config
    local generator_config = target:extraconf("rules", _meta_rule_generators_name)
    
    -- solve config
    local solved_config = {
        scripts = {},
        dep_files = {}
    }

    -- solve scripts
    for _, script_config in ipairs(generator_config.scripts) do
        -- collect import dirs
        local import_dirs = {}
        if script_config.import_dirs then
            for _, dir in ipairs(script_config.import_dirs) do
                table.insert(import_dirs, path.join(target:scriptdir(), dir))
            end 
        end
        
        -- save config
        table.insert(solved_config.scripts, {
            file = path.join(target:scriptdir(), script_config.file),
            private = script_config.private,
            import_dirs = import_dirs,
            use_new_framework = script_config.use_new_framework or false,
        })
    end

    -- solve dep files
    for _, dep_file in ipairs(generator_config.dep_files) do
        local match_files = os.files(path.join(target:scriptdir(), dep_file))
        for __, file in ipairs(match_files) do
            table.insert(solved_config.dep_files, file)
        end
    end

    -- save config
    target:data_set(_meta_data_generators_name, solved_config)
end
function _meta_compile(target, rootdir, metadir, gendir, sourcefile, headerfiles, opt)
    -- generate headers dummy
    local headerfiles = target:data(_meta_data_headers_name)
    if(headerfiles ~= nil and #headerfiles > 0) then
        depend.on_changed(function()
            local verbose = option.get("verbose")
            sourcefile = path.absolute(sourcefile)
            
            -- generate unity source
            local unity_cpp = io.open(sourcefile, "w")
            for _, headerfile in ipairs(headerfiles) do
                headerfile = path.absolute(headerfile)
                local relative_include = path.relative(headerfile, path.directory(sourcefile))
                unity_cpp:print("#include \"%s\"", relative_include)
                if verbose then
                    cprint("${magenta}[%s]: meta.header ${clear}%s", target:name(), path.relative(headerfile))
                end
            end
            unity_cpp:close()

            -- build generated cpp to json
            _meta_compile_command(sourcefile, rootdir, metadir, target, opt)
        end, {dependfile = sourcefile .. ".meta.d", files = headerfiles})
    end
end
function _meta_codegen(target, rootdir, metadir, gendir, sourcefile, headerfiles, opt)
    -- collect framework depend files
    local dep_files = os.files(path.join(metadir, "**.meta"))
    do
        local py_pattern = path.join(os.projectdir(), "tools/meta_codegen/**.py")
        local mako_pattern = path.join(os.projectdir(), "tools/meta_codegen/**.mako")
        for _, file in ipairs(os.files(py_pattern)) do
            table.insert(dep_files, file)
        end
        for _, file in ipairs(os.files(mako_pattern)) do
            table.insert(dep_files, file)
        end
    end
    
    local scripts = {}

    -- extract self generator scripts and dep_files
    if target:data(_meta_data_generators_name) then
        local self_generator_config = target:data(_meta_data_generators_name)
        for _, script_config in ipairs(self_generator_config.scripts) do
            table.insert(scripts, script_config)
        end
        for _, dep_file in ipairs(self_generator_config.dep_files) do
            table.insert(dep_files, dep_file)
        end
    end

    -- extract dep generator scripts and dep_files
    for _, dep in pairs(target:deps()) do
        local dep_generator = dep:data(_meta_data_generators_name)
        if dep_generator then
            -- extract scripts
            for __, script_config in ipairs(dep_generator.scripts) do
                if not script_config.private then
                    table.insert(scripts, script_config)
                end
            end

            -- extract dep_files
            for __, dep_file in ipairs(dep_generator.dep_files) do
                table.insert(dep_files, dep_file)
            end
        end
    end

    -- call codegen scripts
    depend.on_changed(function ()
        _meta_codegen_command(target, scripts, metadir, gendir, opt)
    end, {dependfile = target:dependfile(target:name()..".mako"), files = dep_files})
end

function main()
    if not _g.MetaGenerated then
        local targets = project.ordertargets()

        -- permission
        if (os.host() == "macosx") then
            os.exec("chmod -R 777 ".._meta.program)
        end

        -- parameters
        local opt = opt or {}

        -- collect headers batch
        for _, target in ipairs(targets) do
            _collect_headers_batch(target)
        end

        -- solve generators
        for _, target in ipairs(targets) do
            if target:rule(_meta_rule_generators_name) then
                _solve_generators(target)
            end
        end

        -- compile meta files
        for _, target in ipairs(targets) do
            if target:rule(_meta_rule_codegen_name) then
                -- resume meta compile
                scheduler.co_group_begin(target:name()..".cppgen.meta", function ()
                    meta_target = target:clone()
                    meta_target:set("pcxxheader", nil)
                    meta_target:set("pcheader", nil)
                    scheduler.co_start(_compile_task, _meta_compile, meta_target, opt)
                end)
            end
        end

        -- compile mako templates
        for _, target in ipairs(targets) do
            if target:rule(_meta_rule_codegen_name) then
                -- wait depend metas
                for _, dep in pairs(target:deps()) do
                    if dep:rule(_meta_rule_codegen_name) then
                        scheduler.co_group_wait(dep:name()..".cppgen.meta")
                    end
                end

                -- wait self metas
                scheduler.co_group_wait(target:name()..".cppgen.meta")
                
                -- dispatch
                scheduler.co_group_begin(target:name()..".cppgen.mako", function ()
                    scheduler.co_start(_compile_task, _meta_codegen, target, opt)
                end)
            end
        end

        _g.MetaGenerated = true
    end
end