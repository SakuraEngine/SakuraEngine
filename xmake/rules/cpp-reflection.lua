rule("c++.reflection")
    after_load(function (target, opt)
        import("gen_refl")
        local headerfiles = {}
        local files = target:extraconf("rules", "c++.reflection", "files")
        for _, file in ipairs(files) do
            local p = path.join(target:scriptdir(), file)
            for __, filepath in ipairs(os.files(p)) do
                table.insert(headerfiles, filepath)
            end
        end
        gen_refl(target, headerfiles)
    end)
    on_config(function (target, opt)
        
        import("gen_refl")
        local rootdir = target:extraconf("rules", "c++.reflection", "rootdir")
        local abs_rootdir = path.absolute(path.join(target:scriptdir(), rootdir))
        if has_config("is_msvc") then
            opt = opt or {}
            opt.cl = true
        end
        gen_refl.generate_refl_files(target, abs_rootdir, opt)
        -- add to sourcebatch
        local sourcebatches = target:sourcebatches()
        local gendir = path.join(target:autogendir({root = true}), target:plat(), "reflection/generated")
        if os.exists(gendir) then
            target:add("includedirs", gendir, {public = true})
            target:add("includedirs", path.join(gendir, target:name()))
        end
        local cppfiles = os.files(path.join(gendir, "/**.cpp"))
        for _, file in ipairs(cppfiles) do
            target:add("files", file)
        end
    end)