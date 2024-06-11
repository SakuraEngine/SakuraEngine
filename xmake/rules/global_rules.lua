rule("DisableTargets")
    on_load(function(target)
        local tbl_path = "build/.gens/module_infos/"..target:name()..".table"
        if os.exists(tbl_path) then
            local tbl = io.load(tbl_path)
            local disable = tbl["Disable"]
            if disable then
                target:set("default", false)
            else
                local _default = target:get("default")
                local _ = (_default == nil) or (_default == true)
                if (_default == nil) or (_default == true) then
                    target:set("default", true)
                end
            end
        end
    end)
rule_end()