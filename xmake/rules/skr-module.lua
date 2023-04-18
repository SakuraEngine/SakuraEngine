option("shipping_one_archive")
    set_default(false)
    set_showmenu(true)
    set_description("Toggle to build modules in one executable file.")
option_end()

if(has_config("shipping_one_archive")) then
    add_defines("SHIPPING_ONE_ARCHIVE")
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
    set_sourcekinds("cxx")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "skr.module", "api")
        local version = target:extraconf("rules", "skr.module", "version")
        target:add("values", "skr.module.version", version)
        if(has_config("shipping_one_archive")) then
            target:add("defines","SHIPPING_ONE_ARCHIVE")
            target:add("defines", api.."_IMPL")
        else
            target:add("defines", api.."_SHARED", {public=true})
            target:add("defines", api.."_IMPL")
        end
        -- add codegen headers to include dir
        local gendir = path.join(target:autogendir({root = true}), target:plat(), "codegen")
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
        local headerfile = path.join(target_gendir, "codegen", target:name(), "module.configure.h")

        -- generate dummies
        if (not os.exists(jsonfile)) then
            io.writefile(jsonfile, "")
        end
        if (not os.exists(embedfile)) then
            io.writefile(embedfile, "")
        end
        if (not os.exists(headerfile)) then
            io.writefile(headerfile, "")
        end
        target:add("files", embedfile)

        target:data_set("module.meta.cpp", embedfile)
        target:data_set("module.meta.json", jsonfile)
        target:data_set("module.meta.header", headerfile)
        target:data_set("module.meta.includedir", target_gendir)

        if (target:rule("c++.unity_build")) then
            local unity_build = target:rule("c++.unity_build"):clone()
            unity_build:add("deps", "skr.module", {order = true})
            target:rule_add(unity_build)
        end
        if (target:rule("c.unity_build")) then
            local cunity_build = target:rule("c.unity_build"):clone()
            cunity_build:add("deps", "skr.module", {order = true})
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
        local api = target:extraconf("rules", "skr.module", "api")
        local dep_modules = module_codegen.resolve_skr_module_dependencies(target)
        if has_config("shipping_one_archive") then
            if target:kind() == "binary" then
                for _, dep in pairs(dep_modules) do
                    if is_plat("linux") then
                        target:add("ldflags", "/-Wl,--whole-archive "..dep.." -Wl,--no-whole-archive", {force = true, public = false})
                    elseif is_plat("macosx") then
                        target:add("ldflags", "/-Wl,-force_load "..dep, {force = true, public = false})
                    elseif is_plat("windows") then
                        target:add("ldflags", "/WHOLEARCHIVE:"..dep, {force = true, public = false})
                    end
                end
            end
        end
        -- need build this target?
        module_codegen.skr_module_gen_json(target, target:data("module.meta.json"), dep_modules)
        module_codegen.skr_module_gen_cpp(target, target:data("module.meta.cpp"), dep_modules)
        module_codegen.skr_module_gen_header(target, target:data("module.meta.header"), api)
    end)
    before_buildcmd_files(function(target, batchcmds, sourcebatch, opt)
        -- avoid duplicate linking of object files
        sourcebatch.objectfiles = {}
    end)
rule_end()

rule("skr.static_module")
    on_load(function (target, opt)
        local api = target:extraconf("rules", "skr.static_module", "api")
        target:set("kind", "static")
        if(not has_config("shipping_one_archive")) then
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
    if has_config("shipping_one_archive") then
        set_kind("static")
    else
        set_kind("shared")
    end
    on_load(function (target, opt)
        if(has_config("shipping_one_archive")) then
            for _, dep in pairs(target:get("links")) do
                target:add("links", dep, {public = true})
            end
        end
    end)
    add_rules("skr.module", { api = api, version = engine_version }) 
    opt = opt or {}
    if opt.exception and not opt.noexception then
        set_exceptions("cxx")
    else
        set_exceptions("no-cxx")
    end
end

function executable_module(name, api, version, opt)
    target(name)
    set_kind("binary")
    add_rules("skr.module", { api = api, version = engine_version }) 
    opt = opt or {}
    if opt.exception and not opt.noexception then
        set_exceptions("cxx")
    else
        set_exceptions("no-cxx")
    end
end