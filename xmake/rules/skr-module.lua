option("module_as_objects")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build modules in one executable file.")
option_end()

if(has_config("module_as_objects")) then
    add_defines("MODULE_AS_OBJECTS")
end

rule("skr.shared")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "skr.shared", "api")
        target:set("kind", "shared")
        target:add("defines", api.."_SHARED", {public=true})
        target:add("defines", api.."_IMPL")
    end)
rule_end()

rule("skr.module")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "skr.module", "api")
        local version = target:extraconf("rules", "skr.module", "version")
        target:add("values", "skr.module.version", version)
        if(has_config("module_as_objects")) then
            target:set("kind", "object")
        else
            target:set("kind", "shared")
            target:add("defines", api.."_SHARED", {public=true})
            target:add("defines", api.."_IMPL")
        end
    end)
    after_load(function(target)
        -- imports
        import("core.base.option")
        import("core.project.project")
        import("core.project.depend")
        import("module_codegen")
        -- calculate deps
        local dep_modules = module_codegen.resolve_skr_module_dependencies(target)
        local target_gendir = path.join(target:autogendir({root = true}), target:plat())
        local jsonfile = path.join(target_gendir, "module", "module.configure.json")
        local embedfile = path.join(target_gendir, "module", "module.configure.cpp")
        -- need build this target?
        module_codegen.skr_module_gen_json(target, jsonfile, dep_modules)
        module_codegen.skr_module_gen_cpp(target, embedfile, dep_modules)
    end)
    on_config(function (target)
        local target_gendir = path.join(target:autogendir({root = true}), target:plat())
        local embedfile = path.join(target_gendir, "module", "module.configure.cpp")
    end)
rule_end()

rule("skr.static_module")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "skr.static_module", "api")
        if(has_config("module_as_objects")) then
            target:set("kind", "object")
        else
            target:set("kind", "static")
            target:add("defines", api.."_STATIC", {public=true})
            target:add("defines", api.."_IMPL")
        end
    end)
rule_end()

function public_dependency(dep, version)
    add_deps(dep, {public = true})
    add_values("skr.module.public_dependencies", dep)
    add_values(dep..".version", version)
end

function shared_module(name, api, version, opt)
    target(name)
    add_rules("skr.module", { api = api, version = engine_version }) 
    opt = opt or {}
    if opt.exception and not opt.noexception then
        add_rules("c++.exception")
    else
        add_rules("c++.noexception")
    end
end