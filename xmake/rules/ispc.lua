rule("utils.ispc")
    set_extensions(".ispc")
    add_deps("utils.inherit.links")

    on_config(function (target, opt)
        local header_outputdir =  path.join(path.absolute(target:autogendir()), "rules", "utils", "ispc-headers")
        local obj_outputdir =  path.join(path.absolute(target:autogendir()), "rules", "utils", "ispc-obj")
        os.mkdir(header_outputdir)
        target:add("includedirs", header_outputdir, {public = true})
    end)
    before_buildcmd_file(function (target, batchcmds, sourcefile_ispc, opt)
        import("find_sdk")
        ispc = find_sdk.find_program("ispc")

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
        batchcmds:vrunv(ispc.vexec, 
            {"-O2",
            path.join(os.projectdir(), sourcefile_ispc),
            "-o", obj_path, 
            "-h", header_path,
            target_args,
            "--opt=fast-math"})
        
            print(obj_path)
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