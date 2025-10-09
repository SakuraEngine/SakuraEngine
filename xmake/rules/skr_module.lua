option("shipping_one_archive")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build modules in one executable file.")
option_end()

if has_config("shipping_one_archive") then
    add_defines("SHIPPING_ONE_ARCHIVE")
end

rule("sakura.module")
    on_load(function(target)
        import("skr.analyze")
        
        if xmake.argv()[1] ~= "analyze_project" then
            local analyze_tbl = analyze.load(target:name())
            if analyze_tbl then
                local meta_source = analyze_tbl["Module.MetaSourceFile"]
                if (meta_source ~= "" and os.exists(meta_source)) then
                    target:add("files", meta_source)
                end
            end
        end
    end)
rule_end()

rule("sakura.component")
    on_load(function(target)
        target:add("values", "Sakura.Attributes", "Module.Component")
    end)
    on_config(function (component, opt)
        import("core.project.project")
        local owner_name = component:extraconf("rules", "sakura.component", "owner")
        local owner = project.target(owner_name)
        local owner_api = owner:extraconf("rules", "sakura.dyn_module", "api")
        -- add dep values to owner
        for _, pub_dep in pairs(component:values("sakura.module.public_dependencies")) do
            owner:add("values", "sakura.module.public_dependencies", pub_dep)
        end
        -- insert owner's include dirs
        for _, owner_inc in pairs(owner:get("includedirs")) do
            component:add("includedirs", owner_inc, {public = true})
        end
        -- import api from owner
        if has_config("shipping_one_archive") then
            component:add("defines", owner_api.."_API=", owner_api.."_LOCAL=error")
        else
            component:add("defines", owner_api.."_API=SKR_IMPORT", owner_api.."_LOCAL=error")
        end
    end)
rule_end()

rule("sakura.dyn_module")
    add_deps("sakura.module")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "sakura.dyn_module", "api")
        target:add("values", "Sakura.Attributes", "Module")
        target:add("defines", api.."_IMPL")
        target:add("defines", api.."_EXTERN_C=SKR_EXTERN_C", {public=true})
        if has_config("shipping_one_archive") then
            target:add("defines","SHIPPING_ONE_ARCHIVE")
            target:add("defines", api.."_API=", {public=true})
        else
            target:add("defines", api.."_SHARED", {public=true})
            target:add("defines", api.."_API=SKR_IMPORT", {interface=true})
            target:add("defines", api.."_API=SKR_EXPORT", {public=false})
        end
    end)
    on_config(function(target)
        if has_config("shipping_one_archive") then
            if target:kind() == "binary" then
                local output_dir = vformat("$(builddir)/$(os)/$(arch)/$(mode)")
                for _, dep in pairs(dep_modules) do
                    if is_plat("linux") then
                        target:add("ldflags", "-Wl,--whole-archive "..output_dir.."/lib"..dep..".a -Wl,--no-whole-archive", {force = true, public = false})
                    elseif is_plat("macosx") then
                        target:add("ldflags", "-Wl,-force_load "..output_dir.."/lib"..dep..".a", {force = true, public = false})
                    elseif is_plat("windows") then
                        target:add("ldflags", "/WHOLEARCHIVE:"..dep..".lib", {force = true, public = false})
                    end
                end
            end
        end
    end)
rule_end()

rule("sakura.derived_target")
    after_load(function (target)
        import("core.project.project")
        local owner_name = target:extraconf("rules", "sakura.derived_target", "owner_name")
        local owner = project.target(owner_name)
        if owner:get("default") == false then
            target:set("default", false)
        end
    end)
rule_end()

analyzer_target("Module.MetaSourceFile")
    analyze(function(target, attributes, analyze_ctx)
        if not target:rule("sakura.dyn_module") then
            return "NOT_A_MODULE"
        end

        import("core.project.project")
        import("core.base.json")
        import("skr.utils")

        local gendir = path.join(utils.skr_codegen_dir(target:name()), "codegen")
        local filename = path.join(gendir, target:name()..".configure.cpp")
        if not os.isdir(gendir) then
            os.mkdir(gendir)
        end

        local dep_names = target:values("sakura.module.public_dependencies")
        utils.on_changed(function (change_info)
            -- gather deps
            local dep_modules = {}
            for _, dep in ipairs(target:orderdeps()) do
                if dep:rule("sakura.dyn_module") then
                    table.insert(dep_modules, dep:name())
                end
            end

            -- get basic data
            local pub_deps = target:values("sakura.module.public_dependencies")
            
            -- build module info
            local module_info = {
                api = "0.1.0",
                name = target:name(),
                prettyname = target:name(),
                version = "0.1.0",
                linking = target:kind(),
                author = "unknown",
                url = "https://github.com/SakuraEngine/SakuraEngine",
                license = "EngineDefault (MIT)",
                copyright = "EngineDefault",
                dependencies = json.mark_as_array({})
            }
            for _, dep in pairs(pub_deps) do
                local dep_target = project.target(dep)
                assert(dep_target, "public dependency not found: "..dep)
                assert(dep_target:rule("sakura.module"), "public dependency must be a sakura.module: "..dep_target:name())
                local kind = dep_target:rule("sakura.dyn_module") and "shared" or "static"
                table.insert(module_info.dependencies, {
                    name = dep,
                    version = "0.1.0",
                    kind = kind
                })
            end

            -- build cpp content
            local cpp_content = format(
                "#include \"SkrCore/module/module.hpp\"\n\nSKR_MODULE_METADATA(u8R\"__de___l___im__(%s)__de___l___im__\", %s)",
                json.encode(module_info),
                target:name()
            )
            -- cprint("${green}[%s] module.configure.cpp", target:name())
            io.writefile(filename, cpp_content)
        end, {
            cache_file = utils.depend_file_target(target:name(), "meta_source_file"),
            files = { target:scriptdir(), os.scriptdir() },
            values = dep_names
        })

        return filename
    end)
analyzer_target_end()