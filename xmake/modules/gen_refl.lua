import("core.project.depend")

function cmd_compile(sourcefile, objectfile, target, opt)
    import("core.base.option")
    import("core.base.object")
    import("core.project.depend")
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
    local program, argv = compiler_inst:compargv(sourcefile, objectfile, opt)
    table.insert(argv, "-I"..os.projectdir()..vformat("/SDKs/tools/$(host)/meta-include"))
    import("find_sdk")
    meta = find_sdk.find_program("meta")
    argv2 = {sourcefile, "--output="..sourcefile..".json", "--"}
    for k,v in pairs(argv2) do  
        table.insert(argv, k, v)
    end
    os.execv(meta.vexec, argv)
    return argv
end

function _merge_reflfile(target, sourcefile_refl, headerfiles, opt)
    local dependfile = target:dependfile(sourcefile_refl)
    depend.on_changed(function ()
        -- trace
        vprint("generating.reflection %s", sourcefile_refl)
        -- do merge
        local reflfile = io.open(sourcefile_refl, "w")
        for _, headerfile in ipairs(headerfiles) do
            headerfile = path.absolute(headerfile)
            sourcefile_refl = path.absolute(sourcefile_refl)
            headerfile = path.relative(headerfile, path.directory(sourcefile_refl))
            reflfile:print("#include \"%s\"", headerfile)
        end
        reflfile:close()
        cmd_compile(sourcefile_refl, sourcefile_refl..".o", target)
    end, {dependfile = dependfile, files = headerfiles})
end

function generate_refl_files(target, opt)
    local refl_batch = target:data("reflection.batch")
    if refl_batch then
        for _, batch in ipairs(refl_batch) do
            local sourceinfo = batch
            if sourceinfo then
                local headerfiles = sourceinfo.headerfiles
                local sourcefile_refl = sourceinfo.sourcefile
                if headerfiles then
                    _merge_reflfile(target, sourcefile_refl, headerfiles, opt)
                end
            end
        end
    end
end

function main(target, headerfiles)
    local batchsize = extraconf and extraconf.batchsize
    local extraconf = target:extraconf("rules", "c++.reflection")
    local sourcedir = path.join(target:autogendir({root = true}), target:plat(), "reflection/src")
    local incdir = path.join(target:autogendir({root = true}), target:plat(), "reflection/include")
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
                reflection_batch[id] = sourceinfo
            end
            sourceinfo.headerfiles = sourceinfo.headerfiles or {}
            table.insert(sourceinfo.headerfiles, headerfile)
        end
    end
    -- save unit batch
    target:data_set("reflection.batch", reflection_batch)
end