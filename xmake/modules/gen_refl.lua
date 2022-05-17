import("core.project.depend")
import("private.async.runjobs")
function cmd_compile(sourcefile, rootdir, metadir, target, opt)
    import("core.base.option")
    import("core.base.object")
    import("core.tool.compiler")
    import("core.language.language")
    -- bind target if exists
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
    import("find_sdk")
    local meta = find_sdk.find_program("meta")
    local argv2 = {sourcefile, "--output="..path.absolute(metadir), "--root="..rootdir or path.absolute(target:scriptdir()), "--"}
    for k,v in pairs(argv2) do  
        table.insert(argv, k, v)
    end
    cprint("${cyan}generating.meta ${clear}%s", path.absolute(metadir))
    os.runv(meta.vexec, argv)
    return argv
end

function _merge_reflfile(target, rootdir, metadir, gendir, toolgendir, sourcefile_refl, headerfiles, opt)
    -- generate headers dummy
    local changedfiles = {}
    local generators = {
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
            os.projectdir()..vformat("/tools/codegen/config_resource.py"),
            os.projectdir()..vformat("/tools/codegen/config_resource.cpp.mako"),
        },
        {
            os.projectdir()..vformat("/tools/codegen/rtti.py"),
            os.projectdir()..vformat("/tools/codegen/rtti.cpp.mako"),
            os.projectdir()..vformat("/tools/codegen/rtti.hpp.mako"),
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
    local rebuild = false
    for _, generator in ipairs(generators) do
        local dependfile = target:dependfile(generator[1])
        depend.on_changed(function ()
            rebuild = true
        end, {dependfile = dependfile, files = generator});
    end
    for _, headerfile in ipairs(headerfiles) do
        local dependfile = target:dependfile(headerfile.."meta")
        depend.on_changed(function ()
            table.insert(changedfiles, headerfile);
            cprint("${cyan}reflection.header ${clear}%s", headerfile)
        end, {dependfile = dependfile, files = {headerfile}})
    end
    if rebuild then
        changedfiles = headerfiles
    end
    -- generate dummy .cpp file
    if(#changedfiles > 0) then
        local reflfile = io.open(sourcefile_refl, "w")
        for _, headerfile in ipairs(changedfiles) do
            headerfile = path.absolute(headerfile)
            sourcefile_refl =
            path.absolute(sourcefile_refl)
            headerfile = path.relative(headerfile, path.directory(sourcefile_refl))
            reflfile:print("#include \"%s\"", headerfile)
        end
        reflfile:close()
        -- build generated cpp to json
        cmd_compile(sourcefile_refl, rootdir, metadir, target, opt)
        local depsmeta = {}
        for _, dep in pairs(target:deps()) do
            local depmetadir = path.join(dep:autogendir({root = true}), dep:plat(), "reflection/meta")
            if os.exists(depmetadir) then
                table.insert(depsmeta, depmetadir)
            end
        end
        -- compile jsons to c++
        local api = target:extraconf("rules", "c++.reflection", "api")
        local function task(index)
            local generator = generators[index][1]
            cprint("${cyan}generating.%s${clear} %s", path.filename(generator), path.absolute(metadir))
            import("find_sdk")
            local python = find_sdk.find_program("python3")
            local command = {
                generator,
                path.absolute(metadir), path.absolute(generators[index].gendir or gendir), api or target:name()
            }
            for _, dep in ipairs(depsmeta) do
                table.insert(command, dep)
            end
            os.iorunv(python.program, command)
        end
        runjobs("codegen.cpp", task, {total = #generators})
    end
end

function generate_refl_files(target, rootdir, opt)
    local refl_batch = target:data("reflection.batch")
    if refl_batch then
        for _, sourceinfo in ipairs(refl_batch) do
            if sourceinfo then
                local headerfiles = sourceinfo.headerfiles
                local sourcefile_refl = sourceinfo.sourcefile
                local metadir = sourceinfo.metadir
                local gendir = sourceinfo.gendir
                local toolgendir = sourceinfo.toolgendir
                if headerfiles then
                    _merge_reflfile(target, rootdir, metadir, gendir, toolgendir, sourcefile_refl, headerfiles, opt)
                end
            end
        end
    end
end

function main(target, headerfiles)
    local batchsize = extraconf and extraconf.batchsize or 10
    local extraconf = target:extraconf("rules", "c++.reflection")
    local sourcedir = path.join(target:autogendir({root = true}), target:plat(), "reflection/src")
    local metadir = path.join(target:autogendir({root = true}), target:plat(), "reflection/meta")
    local gendir = path.join(target:autogendir({root = true}), target:plat(), "reflection/generated", target:name())
    local toolgendir = path.join(target:autogendir({root = true}), target:plat(), "tool/generated", target:name())
    local reflection_batch = {}
    local id = 1
    local count = 0
    for idx, headerfile in pairs(headerfiles) do
        local sourcefile_reflection
        if batchsize and count >= batchsize then
            id = id + 1
            count = 0
        end
        sourcefile_reflection = path.join(sourcedir, "reflection_" .. tostring(id) .. ".cpp")
        count = count + 1
        -- batch
        if sourcefile_reflection then
            local sourceinfo = reflection_batch[id]
            if not sourceinfo then
                sourceinfo = {}
                sourceinfo.sourcefile = sourcefile_reflection
                sourceinfo.metadir = metadir
                sourceinfo.gendir = gendir
                sourceinfo.toolgendir = toolgendir
                reflection_batch[id] = sourceinfo
            end
            sourceinfo.headerfiles = sourceinfo.headerfiles or {}
            table.insert(sourceinfo.headerfiles, headerfile)
        end
    end
    -- save unit batch
    target:data_set("reflection.batch", reflection_batch)
end