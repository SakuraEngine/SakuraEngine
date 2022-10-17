import("core.base.option")
import("core.base.object")
import("core.base.scheduler")
import("core.project.depend")
import("core.project.project")
import("core.language.language")
import("core.tool.compiler")
import("find_sdk")

meta = find_sdk.find_program("meta")
python = find_sdk.find_program("python3")

function meta_cmd_compile(sourcefile, rootdir, outdir, target, opt)
    opt = opt or {}
    opt.target = target
    local last = os.time()
    -- load compiler and get compilation command
    local sourcekind = opt.sourcekind
    if not sourcekind and type(sourcefile) == "string" then
        sourcekind = language.sourcekind_of(sourcefile)
    end
    local compiler_inst = compiler.load(sourcekind, opt)
    local program, argv = compiler_inst:compargv(sourcefile, sourcefile..".o", opt)
    if opt.cl then
        table.insert(argv, "--driver-mode=cl")
    end
    table.insert(argv, "-I"..os.projectdir()..vformat("/SDKs/tools/$(host)/meta-include"))
    local argv2 = {sourcefile, "--output="..path.absolute(outdir), "--root="..rootdir or path.absolute(target:scriptdir()), "--"}
    for k,v in pairs(argv2) do  
        table.insert(argv, k, v)
    end
    
    if not opt.quiet then
        cprint("${green}[%s]: compiling.meta ${clear}%s", target:name(), path.relative(outdir))
    end

    os.runv(meta.vexec, argv)

    if not opt.quiet then
        local now = os.time()
        cprint("${green}[%s]: finish.meta ${clear}%s cost ${red}%d seconds", target:name(), path.relative(outdir), now - last)
    end

    return argv
end

function _meta_compile(target, rootdir, metadir, gendir, toolgendir, unityfile, headerfiles, opt)
    -- generate headers dummy
    local changedheaders = target:data("reflection.changedheaders")
    -- generate dummy .cpp file
    if(changedheaders ~= nil and #changedheaders > 0) then
        -- compile jsons to c++
        if (not disable_meta) then
            local unity_cpp = io.open(unityfile, "w")
            for _, headerfile in ipairs(changedheaders) do
                headerfile = path.absolute(headerfile)
                unityfile = path.absolute(unityfile)
                local relative_include = path.relative(headerfile, path.directory(unityfile))
                unity_cpp:print("#include \"%s\"", relative_include)
                cprint("${magenta}[%s]: meta.header ${clear}%s", target:name(), path.relative(headerfile))
            end
            unity_cpp:close()
            -- build generated cpp to json
            meta_cmd_compile(unityfile, rootdir, metadir, target, opt)
            target:data_set("reflection.need_mako", true)
        end
    end
end

function _mako_compile_template(target, mako_generators, use_deps_data, metadir, gendir, opt)
    local api = target:extraconf("rules", "c++.codegen", "api")
    -- compile jsons to c++
    local depsmeta = {}
    for _, dep in pairs(target:deps()) do
        local depmetadir = path.join(dep:autogendir({root = true}), dep:plat(), "reflection/meta")
        table.insert(depsmeta, depmetadir)
    end
    -- compile jsons to c++
    local function template_mako_task(index, depsmeta, opt)
        local generator = mako_generators[index][1]
        local last = os.time()
        if not opt.quiet then
            cprint("${cyan}[%s]: %s${clear} %s", target:name(), path.filename(generator), path.relative(metadir))
        end

        local command = {
            generator,
            path.absolute(metadir), path.absolute(mako_generators[index].gendir or gendir), api or target:name()
        }
        for _, dep in ipairs(depsmeta) do
            table.insert(command, dep)
        end
        os.iorunv(python.program, command)

        if not opt.quiet then
            local now = os.time()
            cprint("${cyan}[%s]: %s${clear} %s cost ${red}%d seconds", target:name(), path.filename(generator), path.relative(metadir), now - last)
        end
    end
    for index, generator in pairs(mako_generators) do
        template_mako_task(index, depsmeta, opt)
    end
end

function _early_mako_compile(target, rootdir, metadir, gendir, toolgendir, unityfile, headerfiles, opt)
    -- generate headers dummy
    local changedheaders = {}
    local early_generators = {
        {
            os.projectdir()..vformat("/tools/codegen/configure.py"),
            os.projectdir()..vformat("/tools/codegen/configure.h.mako"),
        },
    }
    local rebuild = false
    for _, generator in ipairs(early_generators) do
        local dependfile = target:dependfile(generator[1])
        depend.on_changed(function ()
            rebuild = true
        end, {dependfile = dependfile, files = generator});
    end
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
        _mako_compile_template(target, early_generators, false, metadir, gendir, opt)
        target:data_set("reflection.changedheaders", changedheaders)
    end
end

function _weak_mako_compile(target, rootdir, metadir, gendir, toolgendir, unityfile, headerfiles, opt)
    -- generate headers dummy
    local weak_mako_generators = {
        {
            os.projectdir()..vformat("/tools/codegen/typeid.py"),
            os.projectdir()..vformat("/tools/codegen/typeid.hpp.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/config_resource.py"),
            os.projectdir()..vformat("/tools/codegen/config_resource.cpp.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/config_asset.py"),
            os.projectdir()..vformat("/tools/codegen/config_asset.cpp.mako"),
            gendir = toolgendir
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
            os.projectdir()..vformat("/tools/codegen/serialize.py"),
            os.projectdir()..vformat("/tools/codegen/serialize.h.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/static_ctor.py"),
            os.projectdir()..vformat("/tools/codegen/static_ctor.cpp.mako"),
        },
    }
    -- calculate if weak makos need to be rebuild
    local need_mako = target:data("reflection.need_mako")
    local disable_meta = target:extraconf("rules", "c++.codegen", "disable_meta")
    local rebuild = need_mako and not disable_meta
    for _, generator in ipairs(weak_mako_generators) do
        local dependfile = target:dependfile(generator[1])
        depend.on_changed(function ()
            rebuild = true
        end, {dependfile = dependfile, files = generator});
    end
    -- rebuild
    if (rebuild) then
        _mako_compile_template(target, weak_mako_generators, false, metadir, gendir, opt)
    end
end

function _strong_mako_compile(target, rootdir, metadir, gendir, toolgendir, unityfile, headerfiles, opt)
    -- generate headers dummy
    local strong_mako_generators = {
        {
            os.projectdir()..vformat("/tools/codegen/serialize_json.py"),
            os.projectdir()..vformat("/tools/codegen/json_reader.h.mako"),
            os.projectdir()..vformat("/tools/codegen/json_writer.h.mako"),
            os.projectdir()..vformat("/tools/codegen/json_serialize.cpp.mako")
        },
    }
    -- calculate if strong makos need to be rebuild
    local need_mako = target:data("reflection.need_mako")
    local disable_meta = target:extraconf("rules", "c++.codegen", "disable_meta")
    local rebuild = need_mako and not disable_meta
    for _, generator in ipairs(strong_mako_generators) do
        local dependfile = target:dependfile(generator[1])
        depend.on_changed(function ()
            rebuild = true
        end, {dependfile = dependfile, files = generator});
    end
    -- rebuild
    if (rebuild) then
        _mako_compile_template(target, strong_mako_generators, true, metadir, gendir, opt)
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
    local batchsize = extraconf and extraconf.batchsize or 10
    local extraconf = target:extraconf("rules", "c++.codegen")
    local sourcedir = path.join(target:autogendir({root = true}), target:plat(), "reflection/src")
    local metadir = path.join(target:autogendir({root = true}), target:plat(), "reflection/meta")
    local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen", target:name())
    local toolgendir = path.join(target:autogendir({root = true}), target:plat(), "tool/generated", target:name())
    local meta_batch = {}
    local id = 1
    local count = 0
    for idx, headerfile in pairs(headerfiles) do
        local unityfile
        if batchsize and count >= batchsize then
            id = id + 1
            count = 0
        end
        unityfile = path.join(sourcedir, "reflection_" .. tostring(id) .. ".cpp")
        count = count + 1
        -- batch
        if unityfile then
            local sourceinfo = meta_batch[id]
            if not sourceinfo then
                sourceinfo = {}
                sourceinfo.sourcefile = unityfile
                sourceinfo.metadir = metadir
                sourceinfo.gendir = gendir
                sourceinfo.toolgendir = toolgendir
                meta_batch[id] = sourceinfo
            end
            sourceinfo.headerfiles = sourceinfo.headerfiles or {}
            table.insert(sourceinfo.headerfiles, headerfile)
        end
    end
    -- save unit batch
    target:data_set("meta.headers.batch", meta_batch)
end

function compile_task(compile_func, target, opt)
    local refl_batch = target:data("meta.headers.batch")
    local rootdir = target:extraconf("rules", "c++.codegen", "rootdir")
    rootdir = path.absolute(path.join(target:scriptdir(), rootdir))
    if refl_batch then
        for _, sourceinfo in ipairs(refl_batch) do
            if sourceinfo then
                local headerfiles = sourceinfo.headerfiles
                local unityfile = sourceinfo.sourcefile
                local metadir = sourceinfo.metadir
                local gendir = sourceinfo.gendir
                local toolgendir = sourceinfo.toolgendir
                if headerfiles then
                    compile_func(target, rootdir, metadir, gendir, toolgendir, unityfile, headerfiles, opt)
                end
            end
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

    -- parameters
    local opt = {}
    if has_config("is_msvc") then
        opt.cl = true
    end

    -- compile early mako files
    for _, target in ipairs(targets) do
        collect_headers_batch(target)
        if (target:rule("c++.codegen")) then
            scheduler.co_group_begin(target:name()..".cpp-codegen.early_mako", function ()
                scheduler.co_start(compile_task, _early_mako_compile, target, opt)
            end)
        end
    end

    -- compile meta files
    for _, target in ipairs(targets) do
        if (target:rule("c++.codegen")) then
            -- wait early makos
            for _, dep in pairs(target:deps()) do
                if dep:rule("c++.codegen") then
                    scheduler.co_group_wait(dep:name()..".cpp-codegen.early_mako")
                end
            end
            scheduler.co_group_wait(target:name()..".cpp-codegen.early_mako")
            -- resume meta compile
            scheduler.co_group_begin(target:name()..".cpp-codegen.meta", function ()
                scheduler.co_start(compile_task, _meta_compile, target, opt)
            end)
        end
    end

    -- compile mako templates
    for _, target in ipairs(targets) do
        if (target:rule("c++.codegen")) then
            -- wait self metas
            scheduler.co_group_wait(target:name()..".cpp-codegen.meta")
            -- resume weak makos
            scheduler.co_group_begin(target:name()..".cpp-codegen.weak_mako", function ()
                scheduler.co_start(compile_task, _weak_mako_compile, target, opt)
            end)
            -- wait deps metas
            for _, dep in pairs(target:deps()) do
                if dep:rule("c++.codegen") then
                    scheduler.co_group_wait(dep:name()..".cpp-codegen.meta")
                end
            end
            -- resume strong makos
            scheduler.co_group_begin(target:name()..".cpp-codegen.strong_mako", function ()
                scheduler.co_start(compile_task, _strong_mako_compile, target, opt)
            end)
        end
    end
end

function main(targetname)
    if not _g.generated then
        generate_once(targetname)
        _g.generated = true
    end  
end