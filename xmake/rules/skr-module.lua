option("module_as_objects")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build modules in one executable file.")
option_end()

if(has_config("module_as_objects")) then
    add_defines("MODULE_AS_OBJECTS")
end

rule("skr.module")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "skr.module", "api")
        if(has_config("module_as_objects")) then
            target:set("kind", "object")
            -- target:set("targetdir", "/build/windows/x64/debug-")
        else
            target:set("kind", "shared")
            target:add("defines", api.."_SHARED", {public=true})
            target:add("defines", api.."_IMPL")
        end
    end)