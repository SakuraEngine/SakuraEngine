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
    cprint("${green}%s.compiling.meta ${clear}%s", target:name(), path.absolute(outdir))
    os.runv(meta.vexec, argv)
    cprint("${green}%s.finish.meta ${clear}%s", target:name(), path.absolute(outdir))
    return argv
end

function _meta_compile(target, rootdir, metadir, gendir, toolgendir, unityfile, headerfiles, opt)
    -- generate headers dummy
    local changedheaders = {}
    local pre_generators = {
        {
            os.projectdir()..vformat("/tools/codegen/configure.py"),
            os.projectdir()..vformat("/tools/codegen/configure.h.mako"),
        },
    }
    local disable_meta = target:extraconf("rules", "c++.codegen", "disable_meta")
    local rebuild = false
    for _, generator in ipairs(pre_generators) do
        local dependfile = target:dependfile(generator[1])
        depend.on_changed(function ()
            rebuild = true
        end, {dependfile = dependfile, files = generator});
    end
    for _, headerfile in ipairs(headerfiles) do
        local dependfile = target:dependfile(headerfile.."meta")
        depend.on_changed(function ()
            table.insert(changedheaders, headerfile);
            if (not disable_meta) then
                cprint("${magenta}%s.reflection.header ${clear}%s", target:name(), headerfile)
            end
        end, {dependfile = dependfile, files = {headerfile}})
    end
    if rebuild then
        changedheaders = headerfiles
    end
    -- generate dummy .cpp file
    if(#changedheaders > 0) then
        local api = target:extraconf("rules", "c++.codegen", "api")
        -- gen configure file
        local function meta_task(index)
            local pre_generator = pre_generators[index][1]
            cprint("${cyan}%s.%s${clear} %s", target:name(), path.filename(pre_generator), path.absolute(metadir))
            local command = {
                pre_generator,
                path.absolute(metadir), path.absolute(pre_generators[index].gendir or gendir), api or target:name()
            }
            for _, dep in ipairs(depsmeta) do
                table.insert(command, dep)
            end
            os.iorunv(python.program, command)
        end
        -- gen configure file
        for index, pre_generator in pairs(pre_generators) do
            meta_task(index)
        end

        -- compile jsons to c++
        if (not disable_meta) then
            local unity_cpp = io.open(unityfile, "w")
            for _, headerfile in ipairs(changedheaders) do
                headerfile = path.absolute(headerfile)
                unityfile = path.absolute(unityfile)
                headerfile = path.relative(headerfile, path.directory(unityfile))
                unity_cpp:print("#include \"%s\"", headerfile)
            end
            unity_cpp:close()
            -- build generated cpp to json
            meta_cmd_compile(unityfile, rootdir, metadir, target, opt)
            target:data_set("reflection.need_mako", true)
        end
    end
end

function _mako_compile(target, rootdir, metadir, gendir, toolgendir, unityfile, headerfiles, opt)
    -- generate headers dummy
    local mako_generators = {
        {
            os.projectdir()..vformat("/tools/codegen/serialize_json.py"),
            os.projectdir()..vformat("/tools/codegen/json_reader.h.mako"),
            os.projectdir()..vformat("/tools/codegen/json_reader.cpp.mako"),
            os.projectdir()..vformat("/tools/codegen/json_writer.h.mako"),
            os.projectdir()..vformat("/tools/codegen/json_writer.cpp.mako")
        },
        {
            os.projectdir()..vformat("/tools/codegen/serialize.py"),
            os.projectdir()..vformat("/tools/codegen/serialize.h.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/typeid.py"),
            os.projectdir()..vformat("/tools/codegen/typeid.hpp.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/rtti.py"),
            os.projectdir()..vformat("/tools/codegen/rtti.cpp.mako"),
            os.projectdir()..vformat("/tools/codegen/rtti.hpp.mako"),
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
            os.projectdir()..vformat("/tools/codegen/importer.py"),
            os.projectdir()..vformat("/tools/codegen/importer.cpp.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/cooker.py"),
            os.projectdir()..vformat("/tools/codegen/cooker.cpp.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/component.py"),
            os.projectdir()..vformat("/tools/codegen/component.cpp.mako"),
            os.projectdir()..vformat("/tools/codegen/component.hpp.mako"),
        },
    }
    local disable_meta = target:extraconf("rules", "c++.codegen", "disable_meta")
    local need_mako = target:data("reflection.need_mako")
    -- generate dummy .cpp file
    if (need_mako) then
        local api = target:extraconf("rules", "c++.codegen", "api")
        -- compile jsons to c++
        if (not disable_meta) then
            local depsmeta = {}
            for _, dep in pairs(target:deps()) do
                local depmetadir = path.join(dep:autogendir({root = true}), dep:plat(), "reflection/meta")
                if os.exists(depmetadir) then
                    table.insert(depsmeta, depmetadir)
                end
            end
            -- compile jsons to c++
            local function mako_task(index)
                local generator = mako_generators[index][1]
                cprint("${cyan}%s.%s${clear} %s", target:name(), path.filename(generator), path.absolute(metadir))
                local command = {
                    generator,
                    path.absolute(metadir), path.absolute(mako_generators[index].gendir or gendir), api or target:name()
                }
                for _, dep in ipairs(depsmeta) do
                    table.insert(command, dep)
                end
                os.iorunv(python.program, command)
            end
            scheduler.co_group_begin(target:name()..".cpp-codegen.mako", function ()
                for index, generator in pairs(mako_generators) do
                    scheduler.co_start(mako_task, index)
                end
            end)
            scheduler.co_group_wait(target:name()..".cpp-codegen.mako")
        end
    end
end

function collect_batch(target)
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
    target:data_set("meta.batch.data", meta_batch)
end

function meta_compile(target, rootdir, opt)
    collect_batch(target)
    local refl_batch = target:data("meta.batch.data")
    if refl_batch then
        for _, sourceinfo in ipairs(refl_batch) do
            if sourceinfo then
                local headerfiles = sourceinfo.headerfiles
                local unityfile = sourceinfo.sourcefile
                local metadir = sourceinfo.metadir
                local gendir = sourceinfo.gendir
                local toolgendir = sourceinfo.toolgendir
                if headerfiles then
                    _meta_compile(target, rootdir, metadir, gendir, toolgendir, unityfile, headerfiles, opt)
                end
            end
        end
    end
end

function mako_compile(target, rootdir, opt)
    local refl_batch = target:data("meta.batch.data")
    if refl_batch then
        for _, sourceinfo in ipairs(refl_batch) do
            if sourceinfo then
                local headerfiles = sourceinfo.headerfiles
                local unityfile = sourceinfo.sourcefile
                local metadir = sourceinfo.metadir
                local gendir = sourceinfo.gendir
                local toolgendir = sourceinfo.toolgendir
                if headerfiles then
                    _mako_compile(target, rootdir, metadir, gendir, toolgendir, unityfile, headerfiles, opt)
                end
            end
        end
    end
end

function generate_once()
    local targets = project.ordertargets()
    local meta_task = function (target)
        local opt = {}
        if has_config("is_msvc") then
            opt.cl = true
        end
        local rootdir = target:extraconf("rules", "c++.codegen", "rootdir")
        local abs_rootdir = path.absolute(path.join(target:scriptdir(), rootdir))
        local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen")
        target:add("includedirs", gendir, {public = true})
        target:add("includedirs", path.join(gendir, target:name()))

        meta_compile(target, abs_rootdir, opt)
    end

    local mako_task = function (target)
        local opt = {}
        if has_config("is_msvc") then
            opt.cl = true
        end
        local rootdir = target:extraconf("rules", "c++.codegen", "rootdir")
        local abs_rootdir = path.absolute(path.join(target:scriptdir(), rootdir))
        local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen")

        mako_compile(target, abs_rootdir, opt)

        -- add to sourcebatch
        local sourcebatches = target:sourcebatches()
        local cppfiles = os.files(path.join(gendir, "/**.cpp"))
        
        local meta_batch_cpp = {}
        for _, file in ipairs(cppfiles) do
            table.insert(meta_batch_cpp, file)
        end
        target:data_set("meta.batch.cpp", meta_batch_cpp)
    end

    -- compile meta files
    for _, target in ipairs(targets) do
        if (target:rule("c++.codegen")) then
            scheduler.co_group_begin(target:name()..".cpp-codegen.meta", function ()
                scheduler.co_start(meta_task, target)
            end)
        end
    end

    -- compile mako templates
    for _, target in ipairs(targets) do
        if (target:rule("c++.codegen")) then
            scheduler.co_group_begin(target:name()..".cpp-codegen", function ()
                for _, dep in pairs(target:deps()) do
                    if dep:rule("c++.codegen") then
                        scheduler.co_group_wait(dep:name()..".cpp-codegen.meta")
                    end
                end
                scheduler.co_group_wait(target:name()..".cpp-codegen.meta")
                scheduler.co_start(mako_task, target)
            end)
        end
    end
end

function main()
    if not _g.generated then
        generate_once()
        _g.generated = true
    end  
end