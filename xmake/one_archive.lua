-- define rule: one_archive mode
rule("mode.one_archive")
    on_config(function (target)
        -- is one_archive mode now? xmake f -m release
        if is_mode("one_archive") then
            -- set the symbols visibility: hidden
            if not target:get("symbols") and target:kind() ~= "shared" and target:kind() ~= "binary" then
                target:set("symbols", "hidden")
            end
            -- enable optimization
            if not target:get("optimize") then
                if is_plat("android", "iphoneos") then
                    target:set("optimize", "smallest")
                else
                    target:set("optimize", "fastest")
                end
            end
            -- strip all symbols
            if not target:get("strip") then
                target:set("strip", "all")
            end
            -- enable NDEBUG macros to disables standard-C assertions
            target:add("cxflags", "-DNDEBUG")
        end
    end)