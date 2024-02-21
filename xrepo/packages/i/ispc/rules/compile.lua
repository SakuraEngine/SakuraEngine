rule("compile")
    set_extensions(".ispc")
    add_deps("utils.inherit.links")
    on_config(function (target, opt)
        local header_outputdir =  path.join(path.absolute(target:autogendir()), "rules", "utils", "ispc-headers")
        local obj_outputdir =  path.join(path.absolute(target:autogendir()), "rules", "utils", "ispc-obj")
        os.mkdir(target:autogendir())
        os.mkdir(header_outputdir)
        os.mkdir(obj_outputdir)
        target:add("includedirs", header_outputdir, {public = true})
    end)
    before_buildcmd_file(function (target, batchcmds, sourcefile_ispc, opt)
        local binpath = path.join(os.scriptdir(), "..", "bin")
        local wdir = binpath
        local cc = ""
        if is_plat("windows") then
            cc = path.join(binpath, "ispc.exe")
        else
            cc = path.join(binpath, "ispc")
        end
        ispc = { program = cc, wdir = wdir }

        -- permission
        if (os.host() == "macosx") then
            os.exec("chmod -R 777 "..ispc.program)
        end

        ispc_basename = path.filename(sourcefile_ispc):match("..+%..+")
        if (os.arch() == "x86_64" or os.arch() == "x64") then
            target_args = "--target=host"
        else
            target_args = "--target=host"
        end

        local obj_outputdir =  path.join(path.absolute(target:autogendir()), "rules", "utils", "ispc-obj")
        local header_outputdir =  path.join(path.absolute(target:autogendir()), "rules", "utils", "ispc-headers")
        local obj_path =  path.join(obj_outputdir, ispc_basename .. ".obj")
        local obj_avx_path =  path.join(obj_outputdir, ispc_basename .. "_avx.obj")
        local obj_neon_path =  path.join(obj_outputdir, ispc_basename .. "_neon.obj")
        local header_path =  path.join(header_outputdir, ispc_basename .. ".h")
        batchcmds:show_progress(opt.progress, 
            "${color.build.object}compile.ispc %s -> %s", 
            sourcefile_ispc, obj_path)
        batchcmds:mkdir(obj_outputdir)
        batchcmds:mkdir(header_outputdir)
        if ispc.wdir ~= nil then
            batchcmds:cd(ispc.wdir)
        end
        batchcmds:vrunv(ispc.program, 
            {"-O2",
            path.join(os.projectdir(), sourcefile_ispc),
            "-o", obj_path, 
            "-h", header_path,
            target_args,
            "--opt=fast-math"})
        
        assert(os.exists(target:autogendir()))
        table.insert(target:objectfiles(), obj_path)
        if (os.arch() == "x86_64" or os.arch() == "x64") then
            --table.insert(target:objectfiles(), obj_avx_path)
        else
            --table.insert(target:objectfiles(), obj_neon_path)
        end

        batchcmds:add_depfiles(sourcefile_ispc, header_path)
        batchcmds:set_depmtime(os.mtime(obj_path))
        batchcmds:set_depcache(target:dependfile(obj_path))
        batchcmds:set_depmtime(os.mtime(header_path))
        batchcmds:set_depcache(target:dependfile(header_path))
    end)