--[[ rule options
{
    scripts = {
        {
            file = "script.py", -- script file
            private = false, -- if true, generated files will not be exported
            import_dirs = { "import/a", "import/b" }, -- python import dirs
            use_new_framework = true, -- 是否使用新框架 TODO. 过渡用，后面删
        }
    },
    dep_files = { "test/*.py", "test/*.mako" }, -- dep_files
    ...
}
]]
rule("c++.codegen.generators")
    after_load(function (target, opt)
        import("codegen")
        codegen.solve_generators(target)
    end)
rule_end()

rule("c++.codegen.meta")
    set_extensions(".h", ".hpp")
    before_build(function (proxy_target, opt)
        import("core.project.project")
        import("codegen")
        local sourcebatches = proxy_target:sourcebatches()
        if sourcebatches then
            local owner_name = proxy_target:data("c++.codegen.owner")
            local owner = project.target(owner_name)
            local self_sourcebatch = sourcebatches["c++.codegen.meta"]
            local files = self_sourcebatch and self_sourcebatch.sourcefiles or {}
            codegen.collect_headers_batch(owner, files)
            -- compile meta file
            codegen.meta_compile(owner, proxy_target, opt)
        end
    end)
rule_end()

rule("c++.codegen.mako")
    before_build(function (proxy_target, opt)
        import("core.project.project")
        import("codegen")
        -- render mako templates
        local owner_name = proxy_target:data("mako.owner")
        local owner = project.target(owner_name)
        codegen.mako_render(owner, opt)
    end)
rule_end()

rule("c++.codegen.load")
    on_load(function (target, opt)
        if xmake.argv()[1] ~= "analyze_project" then
            -- config
            local codegen_dir = path.join(target:autogendir({root = true}), target:plat(), "codegen")
            local source_file = path.join(codegen_dir, target:name(), "/generated.cpp")

            -- check generated files
            if not os.exists(source_file) then
                local gen_file = io.open(source_file, "w")
                -- gen_file:print("static_assert(false, \"codegen of module "..target:name().." is not completed!\");")
                gen_file:close()
            end

            -- add to target configure
            target:add("files", source_file, { unity_ignored = true })
            target:add("includedirs", codegen_dir, {public = true})

            -- add deps
            local tbl_path = "build/.gens/module_infos/"..target:name()..".table"
            if os.exists(tbl_path) then
                local tbl = io.load(tbl_path)
                local codegen_deps = tbl["Codegen.Deps"]
                for _, codegen_dep in ipairs(codegen_deps) do
                    target:add("deps", dep, { public = false })
                end
            end
        end
    end)
rule_end()

analyzer("Codegen.Deps")
    analyze(function(target, attributes, analyzing)
        local codegen_deps = {}
        local idx = 1
        for __, dep in ipairs(target:orderdeps()) do
            local dep_attrs = analyzing.query_attributes(dep:name())
            if table.contains(dep_attrs, "Codegen.Owner") then
                codegen_deps[idx] = dep:name()..".Codegen"
                idx = idx + 1
            end
        end
        return codegen_deps
    end)
analyzer_end()

function codegen_component(owner, opt)
    target(owner)
        add_rules("c++.codegen.load")
        add_attribute("Codegen.Owner")
        add_deps(owner..".Mako", { public = opt and opt.public or true })
        add_values("c++.codegen.api", opt.api or target:name():upper())
    target_end()

    target(owner..".Mako")
        set_group("01.modules/"..owner.."/codegen")
        set_kind("headeronly")
        add_rules("c++.codegen.mako")
        add_rules("sakura.derived_target", { owner_name = owner })
        add_attribute("Analyze.Ignore")
        set_policy("build.fence", true)
        add_deps(owner..".Meta", { public = true })
        on_load(function (target)
            target:data_set("mako.owner", owner)
        end)

    -- must be declared at the end of this helper function
    target(owner..".Meta")
        set_group("01.modules/"..owner.."/codegen")
        set_kind("headeronly")
        add_rules("c++.codegen.meta")
        add_rules("sakura.derived_target", { owner_name = owner })
        add_attribute("Analyze.Ignore")
        set_policy("build.fence", true)
        on_load(function (target)
            target:data_set("c++.codegen.owner", owner)
            if opt and opt.rootdir then
                opt.rootdir = path.absolute(path.join(target:scriptdir(), opt.rootdir))
            end
            target:data_set("meta.rootdir", opt.rootdir)
            -- TODO: add deps to depended meta targets
            -- ...
        end)
end