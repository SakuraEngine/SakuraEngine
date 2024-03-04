import("core.base.option")
import("core.base.scheduler")
import("core.project.depend")
import("core.project.project")
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
local _meta_rule_codegen_name = "c++.meta.codegen"
local _meta_rule_generators_name = "c++.meta.generators"

-- tools
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
    opt = opt or {
        target = target,
        rawargs = true
    }
    local start_time = os.time()
    local using_msvc = target:toolchain("msvc")
    local using_clang_cl = target:toolchain("clang-cl")

    -- load compiler and get compilation command
    local source_kind = opt.sourcekind
    if not source_kind and type(sourcefile) == "string" then
        source_kind = language.sourcekind_of(sourcefile)
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
    os.runv(meta.program, argv)
    if not opt.quiet then
        cprint(
            "${green}[%s]: finish.meta ${clear}%s cost ${red}%d seconds"
            , target:name()
            , path.relative(outdir)
            , os.time() - start_time
        )
    end
end
function _codegen_command(target, mako_generators, use_deps_data, metadir, gendir, opt)
    -- get config
    local api = target:extraconf("rules", meta_rule, "api")
    local generator = os.projectdir()..vformat("/tools/codegen/codegen.py")
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

    -- config
    table.insert(command, "-config")
    table.insert(command, path.join(target:name(), "module.configure.h"))
    for _, generator in pairs(mako_generators) do
        table.insert(command, generator[1])
    end

    -- call codegen script
    os.iorunv(python.program, command)

    if not opt.quiet then
        local now = os.time()
        cprint("${cyan}[%s]: %s${clear} %s cost ${red}%d seconds", target:name(), path.filename(generator), path.relative(metadir), now - last)
    end
end

-- steps
--  1. collect header batch
--  2. compile headers to meta json
--  3. call codegen scripts
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
    for idx, headerfile in pairs(headerfiles) do
        -- new batch
        if batchsize and file_count >= batchsize then
            batch_id = batch_id + 1
            file_count = 0
        end

        -- add source to batch
        local sourcefile = path.join(sourcedir, "reflection_" .. tostring(id) .. ".cpp")
        if sourcefile then
            local sourceinfo = meta_batch[id]
            if not sourceinfo then
                meta_batch[id] = {
                    sourcefile = sourcefile,
                    metadir = metadir,
                    gendir = gendir,
                    headerfiles = {}
                }
            end
            table.insert(sourceinfo.headerfiles, headerfile)
        end

        file_count = file_count + 1
    end

    -- set data
    target:data_set(_meta_data_batch_name, meta_batch)
    target:data_set(_meta_data_headers_name, headerfiles)
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
    -- collect generators
    local generators = { target:extraconf("rules", _meta_rule_generators_name) }
    for _, dep in pairs(target:deps()) do
        local dep_generator = dep:extraconf("rules", _meta_rule_generators_name)
        if dep_generator then
            table.insert(generators, dep_generator)
        end
    end
end

function main()
    if not _g.MetaGenerated then
        local targets = project.ordertargets()
        
        -- solve generators
        for _, target in ipairs(targets) do
            _meta_codegen(target)
        end

        _g.MetaGenerated = true
    end
end