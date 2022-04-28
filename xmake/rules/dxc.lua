rule("utils.dxc")
    set_extensions(".hlsl")
    on_buildcmd_file(function (target, batchcmds, sourcefile_hlsl, opt)
        import("lib.detect.find_file")
        import("lib.detect.find_program")
        local sdkdir = path.join(os.projectdir(), "build/sdk")
        local dxc = find_program("dxc", {pathes = {sdkdir}})
        local dxcf = find_file("dxc", {sdkdir})
        
        if(dxc == nil) then
            print("dxc not found! under "..sdkdir)
            if(dxcf == nil) then
                print("dxcf not found! under "..sdkdir)
                return
            else
                dxc = dxcf
                vexec = "cd "..sdkdir.." && "..dxc
            end
        else
            vexec = dxc
        end

        -- get target profile
        target_profile = sourcefile_hlsl:match("^.+%.(.+)%.")
        hlsl_basename = path.filename(sourcefile_hlsl):match("(.+)%..+%..+")

        -- hlsl to spv
        local targetenv = target:extraconf("rules", "utils.dxc", "targetenv") or "vulkan1.1"
        local spv_outputdir =  path.join(target:autogendir(), "rules", "utils", "dxc-spv")
        local spvfilepath = path.join(spv_outputdir, hlsl_basename .. ".spv")
        batchcmds:show_progress(opt.progress, "${color.build.object}generating.spirv %s -> %s", sourcefile_hlsl, hlsl_basename .. ".spv")
        batchcmds:mkdir(spv_outputdir)
        batchcmds:vrunv(vexec, 
            {"-Wno-ignored-attributes",
            "-spirv",
            vformat("-fspv-target-env=vulkan1.1"), 
            "-Fo", path.join(os.projectdir(), spvfilepath), 
            "-T", target_profile,
            path.join(os.projectdir(), sourcefile_hlsl)})

        -- hlsl to dxil
        local dxil_outputdir = path.join(target:autogendir(), "rules", "utils", "dxc-dxil")
        local dxilfilepath = path.join(dxil_outputdir, hlsl_basename .. ".dxil")
        batchcmds:show_progress(opt.progress, "${color.build.object}generating.dxil %s -> %s", sourcefile_hlsl, hlsl_basename .. ".dxil")
        batchcmds:mkdir(dxil_outputdir)
        batchcmds:vrunv(vexec, 
            {"-Wno-ignored-attributes", 
            "-Fo ", path.join(os.projectdir(), dxilfilepath), 
            "-T ", target_profile,
            path.join(os.projectdir(), sourcefile_hlsl)})

        -- add deps
        batchcmds:add_depfiles(sourcefile_hlsl)
        batchcmds:set_depmtime(os.mtime(spv_outputdir))
        batchcmds:set_depcache(target:dependfile(spv_outputdir))
        batchcmds:set_depmtime(os.mtime(dxil_outputdir))
        batchcmds:set_depcache(target:dependfile(dxil_outputdir))
    end)
    
    after_build(function(target)
        local spv_path = path.join(target:autogendir(), "rules/utils/dxc-spv")
        local spv_outdir = target:extraconf("rules", "utils.dxc", "spv_outdir")
        if(spv_outdir ~= nil) then
            os.cp(spv_path.."/*.spv", path.join(target:targetdir(), spv_outdir).."/")
        end
        local dxil_path = path.join(target:autogendir(), "rules/utils/dxc-dxil")
        local dxil_outdir = target:extraconf("rules", "utils.dxc", "dxil_outdir")
        if(dxil_outdir ~= nil) then
            os.cp(dxil_path.."/*.dxil", path.join(target:targetdir(), dxil_outdir).."/")
        end
    end)