option("shipping_one_archive")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build modules in one executable file.")
option_end()

if has_config("shipping_one_archive") then
    add_defines("SHIPPING_ONE_ARCHIVE")
end

function public_dependency(dep, version, setting)
    add_deps(dep, {public = true})
    add_values("skr.module.public_dependencies", dep)
    add_values(dep..".version", version)
end

function library_dependency(dep, version, setting)
    add_deps(dep, {public = true})
    add_values(dep..".version", version)
end

rule("skr.module")
rule_end()

rule("skr.component")
    on_config(function (component, opt)
        import("core.project.project")
        local owner_name = component:extraconf("rules", "skr.component", "owner")
        local owner = project.target(owner_name)
        local owner_api = owner:extraconf("rules", "skr.dyn_module", "api") or owner:extraconf("rules", "skr.static_module", "api")
        -- add dep values to owner
        for _, pub_dep in pairs(component:values("skr.module.public_dependencies")) do
            owner:add("values", "skr.module.public_dependencies", pub_dep)
            owner:add("values", pub_dep..".version", component:values(pub_dep..".version"))
        end
        --[[
        -- import deps from owner
        for _, owner_dep in pairs(owner:orderdeps()) do
            local _owner_name = owner_dep:extraconf("rules", "skr.component", "owner") or ""
            if _owner_name ~= owner_name then
                component:add("deps", owner_dep:name(), {public = true})
            end
        end
        ]]--
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

rule("skr.dyn_module")
    add_deps("skr.module")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "skr.dyn_module", "api")
        local version = target:extraconf("rules", "skr.dyn_module", "version")
        target:add("values", "skr.module.version", version)
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
        -- add codegen headers to include dir
        local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen")
        if (not os.exists(gendir)) then
            os.mkdir(gendir)
        end
        target:add("includedirs", gendir, {public = true})

        local target_gendir = path.join(target:autogendir({root = true}), target:plat())
        if (path.is_absolute(target_gendir)) then
            print("Detect incorrect abs path: \""..target_gendir.."\", report this to xmake!")
        end
        target_gendir = path.absolute(target_gendir)
        -- HACK: this absolute path may contains lower-case drive letter on windows
        if (os.host() == "windows") then
            target_gendir = target_gendir:gsub("^%l", string.upper)
        end

        local jsonfile = path.join(target_gendir, "module", "module.configure.json")
        local embedfile = path.join(target_gendir, "module", "module.configure.cpp")

        -- generate dummies
        if (not os.exists(jsonfile)) then
            io.writefile(jsonfile, "")
        end
        if (not os.exists(embedfile)) then
            io.writefile(embedfile, "")
        end
        target:add("files", embedfile)

        target:data_set("module.meta.cpp", embedfile)
        target:data_set("module.meta.json", jsonfile)
        target:data_set("module.meta.includedir", target_gendir)

        if (target:rule("c++.unity_build")) then
            local unity_build = target:rule("c++.unity_build"):clone()
            unity_build:add("deps", "skr.dyn_module", {order = true})
            target:rule_add(unity_build)
        end
        if (target:rule("c.unity_build")) then
            local cunity_build = target:rule("c.unity_build"):clone()
            cunity_build:add("deps", "skr.dyn_module", {order = true})
            target:rule_add(cunity_build)
        end
    end)
    on_config(function(target)
        -- imports
        import("core.base.option")
        import("core.project.project")
        import("core.project.depend")
        import("module_codegen")
        -- calculate deps
        local api = target:extraconf("rules", "skr.dyn_module", "api")
        local dep_modules = module_codegen.resolve_skr_module_dependencies(target)
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
        module_codegen.skr_module_gen_json(target, target:data("module.meta.json"), dep_modules)
        module_codegen.skr_module_gen_cpp(target, target:data("module.meta.cpp"), dep_modules)
    end)
rule_end()

rule("skr.static_library")
    on_load(function (target, opt)
        target:set("kind", "static")
    end)
rule_end()

rule("skr.static_module")
    add_deps("skr.module")
    add_deps("skr.static_library")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "skr.static_module", "api")
        if not has_config("shipping_one_archive") then
            target:add("defines", api.."_API", {public=true})
            target:add("defines", api.."_STATIC", {public=true})
            target:add("defines", api.."_IMPL")
        end
    end)
rule_end()

function static_library(name, api, version, opt)
    target(name)
        set_group("01.libraries/"..name)
        add_rules("skr.static_library", { api = api, version = version }) 
        set_kind("static")
        opt = opt or {}
        if opt.exception and not opt.noexception then
            set_exceptions("cxx")
        else
            set_exceptions("no-cxx")
        end
end

function static_module(name, api, version, opt)
    target(name)
        set_group("01.modules/"..name)
        add_rules("skr.static_module", { api = api, version = version }) 
        set_kind("static")
        opt = opt or {}
        if opt.exception and not opt.noexception then
            set_exceptions("cxx")
        else
            set_exceptions("no-cxx")
        end
end

function shared_module(name, api, version, opt)
    target(name)
        set_group("01.modules/"..name)
        add_rules("skr.dyn_module", { api = api, version = version }) 
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
        set_group("01.modules/"..owner.."/components")
        add_rules("skr.component", { owner = owner })
        set_kind("static")
end


function executable_module(name, api, version, opt)
    target(name)
        set_kind("binary")
        add_rules("skr.dyn_module", { api = api, version = engine_version }) 
        opt = opt or {}
        if opt.exception and not opt.noexception then
            set_exceptions("cxx")
        else
            set_exceptions("no-cxx")
        end
end