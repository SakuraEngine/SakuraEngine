import("core.base.option")
import("core.base.scheduler")
import("core.project.depend")
import("core.project.project")
import("core.language.language")
import("core.tool.compiler")
import("find_sdk")

meta = find_sdk.find_program("meta")
python = find_sdk.find_embed_python() or find_sdk.find_program("python3")

function meta_cmd_compile(sourcefile, rootdir, outdir, target, opt)
    opt = opt or {}
    opt.target = target
    opt.rawargs = true
    local last = os.time()
    -- load compiler and get compilation command
    local sourcekind = opt.sourcekind
    if not sourcekind and type(sourcefile) == "string" then
        sourcekind = language.sourcekind_of(sourcefile)
    end
    local compiler_inst = compiler.load(sourcekind, opt)
    local program, argv = compiler_inst:compargv(sourcefile, sourcefile..".o", opt)
    if opt.msvc then
        table.insert(argv, "--driver-mode=cl")
        --table.insert(argv, "/Tp")
    end
    table.insert(argv, "-I"..os.projectdir()..vformat("/SDKs/tools/$(host)/meta-include"))

    if not opt.quiet then
        cprint("${green}[%s]: compiling.meta ${clear}%s", target:name(), path.relative(outdir))
    end

    if is_host("windows") then
        argv = winos.cmdargv(argv)
    end

    local argv2 = {sourcefile, "--output="..path.absolute(outdir), "--root="..rootdir or path.absolute(target:scriptdir()), "--"}
    for k,v in pairs(argv2) do  
        table.insert(argv, k, v)
    end
    os.runv(meta.program, argv)

    if not opt.quiet then
        local now = os.time()
        cprint("${green}[%s]: finish.meta ${clear}%s cost ${red}%d seconds", target:name(), path.relative(outdir), now - last)
    end
end

function meta_compile(target, rootdir, metadir, gendir, sourcefile, headerfiles, opt)
    -- generate headers dummy
    local changedheaders = target:data("reflection.changedheaders")
    -- generate dummy .cpp file
    if(changedheaders ~= nil and #changedheaders > 0) then
        local verbose = option.get("verbose")
        -- compile jsons to c++
        local unity_cpp = io.open(sourcefile, "w")
        for _, headerfile in ipairs(changedheaders) do
            headerfile = path.absolute(headerfile)
            sourcefile = path.absolute(sourcefile)
            local relative_include = path.relative(headerfile, path.directory(sourcefile))
            unity_cpp:print("#include \"%s\"", relative_include)
            if verbose then
                cprint("${magenta}[%s]: meta.header ${clear}%s", target:name(), path.relative(headerfile))
            end
        end
        unity_cpp:close()
        -- build generated cpp to json
        meta_cmd_compile(sourcefile, rootdir, metadir, target, opt)
        target:data_set("reflection.need_mako", true)
    end
end

function collect_headers_batch(target)
    local headerfiles = {}
    local files = target:extraconf("rules", "c++.codegen", "files")
    for _, file in ipairs(files) do
        local p = path.join(target:scriptdir(), file)
        for __, filepath in ipairs(os.files(p)) do
            table.insert(headerfiles, filepath)
        end
    end
    -- local batchsize = extraconf and extraconf.batchsize or 10
    local batchsize = 999  -- batch size optimization is disabled now
    local extraconf = target:extraconf("rules", "c++.codegen")
    local sourcedir = path.join(target:autogendir({root = true}), target:plat(), "reflection/src")
    local metadir = path.join(target:autogendir({root = true}), target:plat(), "reflection/meta")
    local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen", target:name())
    local meta_batch = {}
    local id = 1
    local count = 0
    for idx, headerfile in pairs(headerfiles) do
        local sourcefile
        if batchsize and count >= batchsize then
            id = id + 1
            count = 0
        end
        sourcefile = path.join(sourcedir, "reflection_" .. tostring(id) .. ".cpp")
        count = count + 1
        -- batch
        if sourcefile then
            local sourceinfo = meta_batch[id]
            if not sourceinfo then
                sourceinfo = {}
                sourceinfo.sourcefile = sourcefile
                sourceinfo.metadir = metadir
                sourceinfo.gendir = gendir
                meta_batch[id] = sourceinfo
            end
            sourceinfo.headerfiles = sourceinfo.headerfiles or {}
            table.insert(sourceinfo.headerfiles, headerfile)
        end
    end
    -- save unit batch
    target:data_set("meta.headers.batch", meta_batch)

    -- generate headers dummy
    local changedheaders = {}
    local rebuild = false
    for _, headerfile in ipairs(headerfiles) do
        local dependfile = target:dependfile(headerfile.."meta")
        depend.on_changed(function ()
            table.insert(changedheaders, headerfile);
        end, {dependfile = dependfile, files = {headerfile}})
    end
    if rebuild then
        changedheaders = headerfiles
    end
    -- generate dummy .cpp file
    if(#changedheaders > 0) then
        target:data_set("reflection.changedheaders", changedheaders)
    end
end

function mako_compile_cmd(target, mako_generators, use_deps_data, metadir, gendir, opt)
    local api = target:extraconf("rules", "c++.codegen", "api")
    local generator = os.projectdir()..vformat("/tools/codegen/codegen.py")
    -- collect deps data
    local depsmeta = {}
    for _, dep in pairs(target:deps()) do
        local depmetadir = path.join(dep:autogendir({root = true}), dep:plat(), "reflection/meta")
        table.insert(depsmeta, depmetadir)
    end
    -- compile jsons to c++
    local last = os.time()
    if not opt.quiet then
        cprint("${cyan}[%s]: %s${clear} %s", target:name(), path.filename(generator), path.relative(metadir))
    end
    
    local command = {
        generator,
        "-root", path.absolute(metadir),
        "-outdir", gendir,
        "-api", api and api:upper() or target:name():upper(),
        "-module", target:name(),
    }
    -- strong order
    table.insert(command, "-includes")
    for _, dep in ipairs(depsmeta) do
        table.insert(command, dep)
    end
    -- config
    table.insert(command, "-config")
    table.insert(command, path.join(target:name(), "module.configure.h"))
    for _, generator in pairs(mako_generators) do
        table.insert(command, generator[1])
    end
    os.iorunv(python.program, command)

    if not opt.quiet then
        local now = os.time()
        cprint("${cyan}[%s]: %s${clear} %s cost ${red}%d seconds", target:name(), path.filename(generator), path.relative(metadir), now - last)
    end
end

function mako_compile(target, rootdir, metadir, gendir, sourcefile, headerfiles, opt)
    -- generate headers dummy
    local mako_generators = {
        {
            os.projectdir()..vformat("/tools/codegen/typeid.py"),
            os.projectdir()..vformat("/tools/codegen/typeid.hpp.mako"),
            os.projectdir()..vformat("/tools/codegen/typeid.cpp.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/component.py"),
            os.projectdir()..vformat("/tools/codegen/component.cpp.mako"),
            os.projectdir()..vformat("/tools/codegen/component.hpp.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/rtti.py"),
            os.projectdir()..vformat("/tools/codegen/rtti.cpp.mako"),
            os.projectdir()..vformat("/tools/codegen/rtti.hpp.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/static_ctor.py"),
            os.projectdir()..vformat("/tools/codegen/static_ctor.cpp.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/serialize_json.py"),
            os.projectdir()..vformat("/tools/codegen/json_serialize.h.mako"),
            os.projectdir()..vformat("/tools/codegen/json_serialize.cpp.mako")
        },
        {
            os.projectdir()..vformat("/tools/codegen/serialize.py"),
            os.projectdir()..vformat("/tools/codegen/binary_serialize.h.mako"),
            os.projectdir()..vformat("/tools/codegen/binary_serialize.cpp.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/luabind.py"),
            os.projectdir()..vformat("/tools/codegen/luabind.h.mako"),
            os.projectdir()..vformat("/tools/codegen/luabind.cpp.mako"),
            os.projectdir()..vformat("/tools/codegen/luabind_intelli.lua.mako"),
            os.projectdir()..vformat("/tools/codegen/luabind_intelli.luau.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/query.py"),
            os.projectdir()..vformat("/tools/codegen/query.hpp.mako"),
            os.projectdir()..vformat("/tools/codegen/query.cpp.mako"),
        },
    }
    -- calculate if strong makos need to be rebuild
    local need_mako = target:data("reflection.need_mako")
    local rebuild = need_mako
    for _, generator in ipairs(mako_generators) do
        local dependfile = target:dependfile(generator[1])
        depend.on_changed(function ()
            rebuild = true
        end, {dependfile = dependfile, files = generator});
    end
    -- rebuild
    if (rebuild) then
        mako_compile_cmd(target, mako_generators, true, metadir, gendir, opt)
    end
end

function compile_task(compile_func, target, opt)
    local refl_batch = target:data("meta.headers.batch") or {}
    local rootdir = target:extraconf("rules", "c++.codegen", "rootdir")
    rootdir = path.absolute(path.join(target:scriptdir(), rootdir))
    for _, sourceinfo in ipairs(refl_batch) do
        sourceinfo = sourceinfo or {}
        local headerfiles = sourceinfo.headerfiles
        if headerfiles then
            compile_func(target, rootdir, sourceinfo.metadir, sourceinfo.gendir, sourceinfo.sourcefile, headerfiles, opt)
        end
    end
end

function generate_fences(targetname)
    import("core.project.rule")
    local all_targets = project.ordertargets()
    local targets = {}
    if (targetname ~= nil and targetname ~= "") then
        table.insert(targets, project.target(targetname))
        local deps = project.target(targetname):deps()
        for _, pending_target in pairs(all_targets) do
            -- ensure needed
            for __, dep in pairs(deps) do
                if (pending_target:name() == dep:name()) then
                    table.insert(targets, pending_target)
                end
            end
        end
    else
        targets = all_targets
    end

    -- inject fence rules
    for _, target in ipairs(targets) do
        local need_fence = false
        for __, dep in pairs(target:deps()) do
            if (dep:rule("c++.codegen")) then
                -- inject fence rule for dependency with c++.codegen rule
                need_fence = true
            end
        end
        if (target:rule("c++.codegen")) then
            need_fence = true
        end
        if (need_fence) then
            local fence = project.rule("c++.codegen.fence") or rule.rule("c++.codegen.fence")
            target:rule_add(fence)
        end
    end
end
    
function generate_once(targetname)
    local all_targets = project.ordertargets()
    local targets = {}
    if (targetname ~= nil and targetname ~= "") then
        table.insert(targets, project.target(targetname))
        local deps = project.target(targetname):deps()
        for _, pending_target in pairs(all_targets) do
            -- ensure needed
            for __, dep in pairs(deps) do
                if (pending_target:name() == dep:name()) then
                    table.insert(targets, pending_target)
                end
            end
        end
    else
        targets = all_targets
    end

    -- permission
    if (os.host() == "macosx") then
        os.exec("chmod -R 777 "..meta.program)
    end
    
    -- parameters
    local opt = opt or {}
    if has_config("is_msvc") then
        opt.msvc = true
    end

    -- collect header batch
    for _, target in ipairs(targets) do
        collect_headers_batch(target)
    end

    -- compile meta files
    for _, target in ipairs(targets) do
        if (target:rule("c++.codegen")) then
            -- resume meta compile
            scheduler.co_group_begin(target:name()..".cpp-codegen.meta", function ()
                scheduler.co_start(compile_task, meta_compile, target, opt)
            end)
        end
    end

    -- compile mako templates
    for _, target in ipairs(targets) do
        if (target:rule("c++.codegen")) then
            scheduler.co_group_begin(target:name()..".cpp-codegen", function ()
                -- wait self metas
                for _, dep in pairs(target:deps()) do
                    if dep:rule("c++.codegen") then
                        scheduler.co_group_wait(dep:name()..".cpp-codegen.meta")
                    end
                end
                scheduler.co_group_wait(target:name()..".cpp-codegen.meta")
                scheduler.co_start(compile_task, mako_compile, target, opt)
            end)
        end
    end

    if(not has_config("use_async_codegen")) then
        -- wait all
        for _, target in ipairs(targets) do
            if (target:rule("c++.codegen")) then
                scheduler.co_group_wait(target:name()..".cpp-codegen")
            end
        end
        print("wait all sync codegen.")
    else
        print("use async codegen.")
    end
end

function main(targetname)
    if not _g.generated then
        generate_once(targetname)
        _g.generated = true
    end  
end