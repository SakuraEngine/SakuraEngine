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
        -- calculate deps
        local target_gendir = path.join(target:autogendir({root = true}), target:plat(), "module")
        local jsonfile = path.join(target_gendir, "module.configure.json")
        local pub_deps = target:values("skr.module.public_dependencies")
        local calculated_flatten = {}
        for _, dep in ipairs(pub_deps) do
            local dep_target = project.target(dep)
            if dep_target:rule("skr.module") then
                calculated_flatten[dep] = dep_target
            end
            for _, depdep in ipairs(dep_target:orderdeps()) do
                if depdep:rule("skr.module") then
                    calculated_flatten[depdep:name()] = depdep
                end
            end
        end
        local dep_modules = {}
        for _, dep in ipairs(target:orderdeps()) do
            if dep:rule("skr.module") then
                table.insert(dep_modules, dep:name())
                assert(calculated_flatten[dep:name()], 
                    "linked skr.module "..dep:name().." is not public dependency of "..target:name()..", please add it to public/private_dependencies")
            end
        end

        -- need build this target?
        local last = os.time()
        local dependfile = target:dependfile(jsonfile)
        local dependinfo = option.get("rebuild") and {} or (depend.load(dependfile) or {})
        if not depend.is_changed(dependinfo, {lastmtime = os.mtime(jsonfile), values = dep_modules, files = jsonfile}) then
            return
        end
        -- start rebuild json
        -- print("generate target metadata: "..target:name())
        local self_version = target:values("skr.module.version")
        assert(self_version, "module version not found: "..target:name())
        local jsonf = io.open(jsonfile, "w")
        local json_content = "{\n\"module\": {\n"
        json_content = json_content.."\t\"name\": \"" .. target:name().."\",\n"
        json_content = json_content.."\t\"prettyname\": \"" .. target:name().."\",\n"
        json_content = json_content.."\t\"version\": \""..self_version.."\",\n"
        json_content = json_content.."\t\"linking\": \""..target:kind().."\",\n"
        json_content = json_content.."\t\"author\": \"unknown\",\n"
        json_content = json_content.."\t\"url\": \"https://github.com/SakuraEngine/SakuraEngine\",\n"
        json_content = json_content.."\t\"license\": \"EngineDefault (MIT)\",\n"
        json_content = json_content.."\t\"copyright\": \"EngineDefault\",\n"
        -- dep body
        json_content = json_content.."\t\"public_dependencies\": [\n "
        for _, dep in pairs(pub_deps) do
            local deptarget = project.target(dep)
            assert(deptarget:rule("skr.module"), "public dependency must be a skr.module: "..deptarget:name())
            local depversion = target:values(dep..".version")
            json_content = json_content.."\t\t{ \"name\":\""..dep.."\", \"version\": \""..depversion.."\" },\n"
        end
        json_content = json_content.sub(json_content, 1, -3)
        json_content = json_content.."\n\t]\n}\n}"
        -- end dep body
        -- write json & update files and values to the dependent file
        jsonf:print(json_content)
        dependinfo.files = {jsonfile}
        dependinfo.values = dep_modules
        depend.save(dependinfo, dependfile)
        -- output time
        if not option.get("quiet") then
            local now = os.time()
            cprint("${green}[%s]: skr.module.exportjson ${clear}%s cost ${red}%d seconds", target:name(), path.relative(jsonfile), now - last)
        end
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