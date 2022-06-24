option("module_as_objects")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build modules in one executable file.")
option_end()

rule("skr.module")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "skr.module", "api")
        if(has_config("module_as_objects")) then
            target:set("kind", "object")
        else
            target:set("kind", "shared")
            target:add("defines", api.."_SHARED", {public=true})
            target:add("defines", api.."_IMPL")
        end
    end)