rule("c++.reflection")
    after_load(function (target)
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
    before_build(function (target, opt)
        import("gen_refl")
        local rootdir = target:extraconf("rules", "c++.reflection", "rootdir")
        local abs_rootdir = path.absolute(path.join(target:scriptdir(), rootdir))
        gen_refl.generate_refl_files(target, abs_rootdir, opt)
    end)