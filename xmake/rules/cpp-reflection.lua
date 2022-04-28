rule("c++.reflection")
    after_load(function (target)
        import("gen_refl")
        local sourcebatches = target:sourcebatches()
        if sourcebatches then
            local sourcebatch = sourcebatches["c++.build"]
            local headerfiles = target:headerfiles()
            local include_dirs = target:get("includedirs")
            if sourcebatch then
                for _, include_dir in ipairs(include_dirs) do
                    for __, filepath in ipairs(os.files(include_dir.."/*.h")) do
                        table.insert(headerfiles, filepath)
                    end
                end
                gen_refl(target, headerfiles, sourcebatch)
            end
        end
    end)
    before_build(function (target, opt)
        import("gen_refl")
        local sourcebatches = target:sourcebatches()
        if sourcebatches then
            local sourcebatch = sourcebatches["c++.build"]
            if sourcebatch then
                gen_refl.generate_refl_files(target, sourcebatch, opt)
            end
        end
    end)