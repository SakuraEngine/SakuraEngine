rule("c++.codegen")
    after_load(function (target, opt)
        import("meta_codegen")
        local headerfiles = {}
        local files = target:extraconf("rules", "c++.codegen", "files")
        for _, file in ipairs(files) do
            local p = path.join(target:scriptdir(), file)
            for __, filepath in ipairs(os.files(p)) do
                table.insert(headerfiles, filepath)
            end
        end
        meta_codegen(target, headerfiles)
    end)
    on_config(function (target, opt)
        import("meta_codegen")
        local rootdir = target:extraconf("rules", "c++.codegen", "rootdir")
        local abs_rootdir = path.absolute(path.join(target:scriptdir(), rootdir))
        if has_config("is_msvc") then
            opt = opt or {}
            opt.cl = true
        end
        -- generate code files
        local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen")
        target:add("includedirs", gendir, {public = true})
        target:add("includedirs", path.join(gendir, target:name()))
        meta_codegen.generate_code_files(target, abs_rootdir, opt)
        -- add to sourcebatch
        local sourcebatches = target:sourcebatches()
        local cppfiles = os.files(path.join(gendir, "/**.cpp"))
        for _, file in ipairs(cppfiles) do
            target:add("files", file)
        end
    end)