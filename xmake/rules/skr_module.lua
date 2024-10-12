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
        if xmake.argv()[1] ~= "analyze_project" then
            local tbl_path = "build/.gens/module_infos/"..target:name()..".table"
            if os.exists(tbl_path) then
                local tbl = io.load(tbl_path)
                local meta_source = tbl["Module.MetaSourceFile"]
                if (meta_source ~= "") then
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
            owner:add("values", pub_dep..".version", component:values(pub_dep..".version"))
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
        local version = target:extraconf("rules", "sakura.dyn_module", "version")
        target:add("values", "Sakura.Attributes", "Module")
        target:add("values", "sakura.module.version", version)
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
                local output_dir = vformat("$(buildir)/$(os)/$(arch)/$(mode)")
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

function shared_module(name, api, version, opt)
    target(name)
        set_group("01.modules/"..name)
        add_rules("PickSharedPCH")
        add_rules("sakura.dyn_module", { api = api, version = version }) 
        if has_config("shipping_one_archive") then
            set_kind("static")
        else
            set_kind("shared")
        end
        on_load(function (target, opt)
            if has_config("shipping_one_archive") then
                for _, dep in pairs(target:get("links")) do
                    target:add("links", dep, {public = true})
                end
            end
        end)
        opt = opt or {}
        if opt.exception and not opt.noexception then
            set_exceptions("cxx")
        else
            set_exceptions("no-cxx")
        end
end

function static_component(name, owner, opt)
    target(owner)
        add_deps(name, { public = opt and opt.public or true })
    target_end()
    
    target(name)
        set_kind("static")
        set_group("01.modules/"..owner.."/components")
        add_rules("sakura.component", { owner = owner })
end

function executable_module(name, api, version, opt)
    target(name)
        set_kind("binary")
        add_rules("PickSharedPCH")
        add_rules("sakura.dyn_module", { api = api, version = engine_version }) 
        local opt = opt or {}
        if opt.exception and not opt.noexception then
            set_exceptions("cxx")
        else
            set_exceptions("no-cxx")
        end
end

function public_dependency(dep, version, setting)
    add_deps(dep, {public = true})
    add_values("sakura.module.public_dependencies", dep)
    add_values(dep..".version", version)
end

analyzer_target("Module.MetaSourceFile")
    analyze(function(target, attributes, analyzing)
        if not target:rule("sakura.dyn_module") then
            return "NOT_A_MODULE"
        end

        import("core.project.depend")
        import("core.project.project")

        local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen")
        local filename = path.join(gendir, "module", "module.configure.cpp")
        local dep_names = target:values("sakura.module.public_dependencies")
        depend.on_changed(function()
            -- gather deps
            local dep_modules = {}
            for _, dep in ipairs(target:orderdeps()) do
                if dep:rule("sakura.dyn_module") then
                    table.insert(dep_modules, dep:name())
                end
            end

            -- start rebuild json
            local self_version = target:values("sakura.module.version")
            local pub_deps = target:values("sakura.module.public_dependencies")
            assert(self_version, "module version not found: "..target:name())
            local delim = "__de___l___im__("
            local delim2 = ")__de___l___im__"
            local cpp_content = "#include \"SkrCore/module/module.hpp\"\n\nSKR_MODULE_METADATA(u8R\""..delim.."\n{\n"
            cpp_content = cpp_content.."\t\"api\": \"" .. self_version.."\",\n"
            cpp_content = cpp_content.."\t\"name\": \"" .. target:name() .."\",\n"
            cpp_content = cpp_content.."\t\"prettyname\": \"" .. target:name() .."\",\n"
            cpp_content = cpp_content.."\t\"version\": \"".. self_version .."\",\n"
            cpp_content = cpp_content.."\t\"linking\": \"".. target:kind() .."\",\n"
            cpp_content = cpp_content.."\t\"author\": \"unknown\",\n"
            cpp_content = cpp_content.."\t\"url\": \"https://github.com/SakuraEngine/SakuraEngine\",\n"
            cpp_content = cpp_content.."\t\"license\": \"EngineDefault (MIT)\",\n"
            cpp_content = cpp_content.."\t\"copyright\": \"EngineDefault\",\n"
            -- dep body
            cpp_content = cpp_content.."\t\"dependencies\": [\n "
            for _, dep in pairs(pub_deps) do
                local deptarget = project.target(dep)
                assert(deptarget:rule("sakura.module"), "public dependency must be a sakura.module: "..deptarget:name())
                local depversion = target:values(dep..".version")
                local kind = deptarget:rule("sakura.dyn_module") and "shared" or "static"
                cpp_content = cpp_content.."\t\t{ \"name\":\""..dep.."\", \"version\": \""..depversion.."\", \"kind\": \""..kind.."\" },\n"
            end
            cpp_content = cpp_content.sub(cpp_content, 1, -3)
            cpp_content = cpp_content.."\n\t]\n}\n"..delim2.."\", "..target:name()..")"
            -- end dep body
            -- write json & update files and values to the dependent file
            io.writefile(filename, cpp_content)
        end, {
            dependfile = target:dependfile("Module.MetaSourceFile"), 
            files = { target:scriptdir() },
            values = dep_names
        })

        return filename
    end)
analyzer_target_end()