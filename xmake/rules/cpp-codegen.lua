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
        meta_codegen.generate_once()
    end)