import("core.base.option")
import("core.base.json")
import("core.project.depend")
import("core.language.language")
import("core.tool.compiler")
import("find_sdk")

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

-- steps
--  1. collect header batch
--  2. solve generators
--  3. compile headers to meta json
--  4. call codegen scripts
function collect_headers_batch(target, headerfiles)
    -- get config
    local batchsize = 999 -- batch size optimization is disabled now
    local sourcedir = path.join(target:autogendir({root = true}), target:plat(), "meta_src")
    local metadir = path.join(target:autogendir({root = true}), target:plat(), "meta_database")
    local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen", target:name())

    -- build batches
    local sourcefile = path.join(sourcedir, "reflection_batch" .. ".cpp")
    local meta_batch = {
        sourcefile = sourcefile,
        metadir = metadir,
        gendir = gendir,
        headerfiles = {}
    }
    local file_count = 0
    for _, headerfile in pairs(headerfiles) do
        -- add source to batch
        table.insert(meta_batch.headerfiles, headerfile)
        file_count = file_count + 1
    end

    -- set data
    target:data_set(_meta_data_batch_name, meta_batch)
    target:data_set(_meta_data_headers_name, headerfiles)
end

function solve_generators(target)
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

function _meta_compile(target, proxy_target, batchcmds, opt)
    local headerfiles = target:data(_meta_data_headers_name)
    local batchinfo = target:data(_meta_data_batch_name)
    local sourcefile = batchinfo.sourcefile
    local outdir = batchinfo.metadir
    local rootdir = path.absolute(proxy_target:data("meta.rootdir")) or path.absolute(target:scriptdir())

    opt = opt or {}
    opt.target = target
    opt.rawargs = true

    local start_time = os.time()
    local using_msvc = target:toolchain("msvc")
    local using_clang_cl = target:toolchain("clang-cl")

    -- fix macosx include solve bug
    if is_plat("macosx") then
        for _, dep_target in pairs(target:orderdeps()) do
            local dirs = dep_target:get("includedirs")
            target:add("includedirs", dirs)
        end
    end

    -- load compiler and get compilation command
    local sourcekind = opt.sourcekind
    if not sourcekind and type(sourcefile) == "string" then
        sourcekind = language.sourcekind_of(sourcefile)
    end
    local compiler_inst = compiler.load(sourcekind, opt)
    local program, argv = compiler_inst:compargv(sourcefile, sourcefile..".o", opt)
    if using_msvc or using_clang_cl then
        table.insert(argv, "--driver-mode=cl")
    end
    table.insert(argv, "-I"..os.projectdir()..vformat("/SDKs/tools/$(host)/meta-include"))

    -- hack: insert a placeholder to avoid the case where (#argv < limit) and (#argv + #argv2 > limit)
    local argv2 = {
        sourcefile, 
        "--output="..path.absolute(outdir), 
        "--root="..rootdir,
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
    if option.get("verbose") then
        batchcmds:show(
            "${green}[%s]: compiling.meta ${clear}%ss\n%s"
            , target:name()
            , path.relative(outdir)
            , command
        )
    elseif not opt.quiet then
        batchcmds:show(
            "${green}[%s]: compiling.meta ${clear}%ss"
            , target:name()
            , path.relative(outdir)
            -- , command
        )
    end

    -- compile meta source
    batchcmds:runv(_meta.program, argv)
    if not opt.quiet then
        batchcmds:show(
            "${green}[%s]: finish.meta ${clear}%s cost ${red}%d seconds"
            , target:name()
            , path.relative(outdir)
            , os.time() - start_time
        )
    end

    -- add deps
    batchcmds:add_depfiles(headerfiles)
end

function meta_compile(target, proxy_target, batchcmds, opt)
    local headerfiles = target:data(_meta_data_headers_name)
    local batchinfo = target:data(_meta_data_batch_name)
    local sourcefile = batchinfo.sourcefile

    -- generate headers dummy
    if(headerfiles ~= nil and #headerfiles > 0) then
        depend.on_changed(function()
            local verbose = option.get("verbose")
            sourcefile = path.absolute(sourcefile)
            
            -- generate unity source
            local unity_cpp = io.open(sourcefile, "w")
            local content_string = ""
            for _, headerfile in ipairs(headerfiles) do
                headerfile = path.absolute(headerfile)
                local relative_include = path.relative(headerfile, path.directory(sourcefile))
                relative_include = relative_include:gsub("\\", "/")
                content_string = content_string .. "#include \"" .. relative_include .. "\"\n"
                if verbose then
                    cprint("${magenta}[%s]: meta.header ${clear}%s", target:name(), path.relative(headerfile))
                end
            end
            unity_cpp:print(content_string)
            unity_cpp:close()

            -- build generated cpp to json
            _meta_compile(target, proxy_target, batchcmds, opt)
        end, {dependfile = sourcefile .. ".meta.d", files = headerfiles})
    end
end

----------------------------------------------------------------------------------------------------------------------------------------------

function _mako_render(target, proxy_target, batchcmds, scripts, dep_files, opt)
    -- get config
    local batchinfo = target:data(_meta_data_batch_name)
    local headerfiles = target:data(_meta_data_headers_name)
    local sourcefile = batchinfo.sourcefile
    local metadir = batchinfo.metadir
    local gendir = batchinfo.gendir

    local api = proxy_target:data("meta.api")
    local generate_script = os.projectdir()..vformat("/tools/meta_codegen/codegen.py")
    local start_time = os.time()

    if not opt.quiet then
        batchcmds:show("${cyan}[%s]: %s${clear} %s", target:name(), path.filename(generate_script), path.relative(metadir))
    end

    -- config
    local config = {
        output_dir = path.absolute(gendir),
        main_module = {
            module_name = target:name(),
            meta_dir = path.absolute(metadir),
            api = api and api:upper() or target:name():upper(),
        },
    }

    -- collect include modules
    config.include_modules = {}
    for _, dep_target in pairs(target:deps()) do
        local dep_api = dep_target:data("meta.api")
        table.insert(config.include_modules, {
            module_name = dep_target:name(),
            meta_dir = path.absolute(path.join(dep_target:autogendir({root = true}), dep_target:plat(), "meta_database")),
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
        batchcmds:show(
            "[%s] python %s"
            , target:name()
            , table.concat(command, " ")
        )
    end

    -- call codegen script
    batchcmds:execv(_python.program, command)

    -- local result = os.iorunv(_python.program, command)
    --if result and #result > 0 then
        --print(result)
    --end

    if not opt.quiet then
        batchcmds:show(
            "${cyan}[%s]: %s${clear} %s cost ${red}%d seconds"
            , target:name()
            , path.filename(generate_script)
            , path.relative(metadir)
            , os.time() - start_time)
    end

    -- add deps
    batchcmds:add_depfiles(dep_files)

    batchcmds:set_depmtime(os.mtime(gendir))
    batchcmds:set_depcache(target:dependfile(gendir))
end

function mako_render(target, proxy_target, batchcmds, opt)
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

    -- collect target depend files
    -- TODO. use config comapre
    table.insert(dep_files, path.join(target:scriptdir(), "xmake.lua"))
    for _, dep_target in ipairs(target:deps()) do
        target.insert(dep_files, path.join(dep_target:scriptdir(), "xmake.lua"))
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
    _mako_render(target, proxy_target, batchcmds, scripts, dep_files, opt)
end