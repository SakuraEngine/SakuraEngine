import("core.project.depend")

function _merge_unityfile(target, sourcefile_unity, sourcefiles, opt)
    local dependfile = target:dependfile(sourcefile_unity)
    depend.on_changed(function ()

        -- trace
        vprint("generating.unityfile %s", sourcefile_unity)

        -- do merge
        local uniqueid = target:data("unity_build.uniqueid")
        local unityfile = io.open(sourcefile_unity, "w")
        for _, sourcefile in ipairs(sourcefiles) do
            sourcefile = path.absolute(sourcefile)
            sourcefile_unity = path.absolute(sourcefile_unity)
            sourcefile = path.relative(sourcefile, path.directory(sourcefile_unity))
            if uniqueid then
                unityfile:print("#define %s %s", uniqueid, "unity_" .. hash.uuid():split("-", {plain = true})[1])
            end
            unityfile:print("#include \"%s\"", sourcefile)
            if uniqueid then
                unityfile:print("#undef %s", uniqueid)
            end
        end
        unityfile:close()

    end, {dependfile = dependfile, files = sourcefiles})
end

function generate_refl_files(target, sourcebatch, opt)
    local unity_batch = target:data("unity_build.unity_batch." .. sourcebatch.rulename)
    if unity_batch then
        for _, sourcefile_unity in ipairs(sourcebatch.sourcefiles) do
            local sourceinfo = unity_batch[sourcefile_unity]
            if sourceinfo then
                local sourcefiles = sourceinfo.sourcefiles
                if sourcefiles then
                    _merge_unityfile(target, sourcefile_unity, sourcefiles, opt)
                end
            end
        end
    end
end

function main(target, headerfiles, sourcebatch)
    local extraconf = target:extraconf("rules", sourcebatch.sourcekind == "cxx" and "c++.reflection" or "c.reflection")
    local uniqueid = extraconf and extraconf.uniqueid
    local sourcefiles = {}
    local objectfiles = {}
    local dependfiles = {}
    local sourcedir = path.join(target:autogendir({root = true}), target:plat(), "reflection/src")
    local incdir = path.join(target:autogendir({root = true}), target:plat(), "reflection/include")
    for idx, sourcefile in pairs(sourcebatch.sourcefiles) do
        local objectfile = sourcebatch.objectfiles[idx]
        local dependfile = sourcebatch.dependfiles[idx]
        local fileconfig = target:fileconfig(sourcefile)

        table.insert(sourcefiles, sourcefile)
        table.insert(objectfiles, objectfile)
        table.insert(dependfiles, dependfile)

        --print(sourcefile.." -meta-> "..sourcedir.."/"..path.basename(sourcefile)..".gen.cpp")
    end
    for idx, headerfile in pairs(headerfiles) do
        --print(headerfile.." -meta-> "..incdir.."/"..path.basename(headerfile)..".gen.hpp")
    end

    sourcebatch.sourcefiles = sourcefiles
    sourcebatch.objectfiles = objectfiles
    sourcebatch.dependfiles = dependfiles

    -- save unit batch
    target:add("includedirs", incdir)
    target:data_set("reflection.uniqueid", uniqueid)
end