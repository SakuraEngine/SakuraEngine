rule("utils.ispc")
    set_extensions(".ispc")
    before_buildcmd_file(function (target, batchcmds, sourcefile_ispc, opt)
        import("find_sdk")
        ispc = find_sdk.find_program("ispc")

        ispc_basename = path.filename(sourcefile_ispc):match("..+%..+")
        if (os.arch() == "x86_64" or os.arch() == "x64") then
            target_args = "--target=avx"
        else
            target_args = "--target=neon"
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
        batchcmds:vrunv(ispc.vexec, 
            {"-O2",
            path.join(os.projectdir(), sourcefile_ispc),
            "-o", obj_path, 
            "-h", header_path,
            target_args,
            "--opt=fast-math"})

        target:add("includedirs", header_outputdir)
        target:add("obj", obj_path)
        if (os.arch() == "x86_64" or os.arch() == "x64") then
            target:add("obj", obj_avx_path)
        else
            target:add("obj", obj_neon_path)
        end

        batchcmds:add_depfiles(sourcefile_ispc)
        batchcmds:set_depmtime(os.mtime(obj_path))
        batchcmds:set_depcache(target:dependfile(obj_path))
        batchcmds:set_depmtime(os.mtime(header_path))
        batchcmds:set_depcache(target:dependfile(header_path))
    end)