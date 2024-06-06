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
rule("c++.meta.generators")
    after_load(function (target, opt)
        import("codegen")
        codegen.solve_generators(target)
    end)
rule_end()

rule("codegen.headers")
    set_extensions(".h", ".hpp")
    before_build(function (proxy_target, opt)
        import("core.project.project")
        import("codegen")

        local sourcebatches = proxy_target:sourcebatches()
        if sourcebatches then
            local owner_name = proxy_target:data("meta.owner")
            local owner = project.target(owner_name)
            local files = sourcebatches["codegen.headers"].sourcefiles
            codegen.collect_headers_batch(owner, files)

            -- compile meta file
            codegen.meta_compile(owner, proxy_target, opt)

            -- render mako templates
            codegen.mako_render(owner, proxy_target, opt)
        end
    end)
rule_end()

analyzer("Codegen.Deps")
    analyze(function(target, attributes, analyzing)
        local codegen_deps = {}
        for __, dep in pairs(target:deps()) do
            local dep_attrs = analyzing.query_attributes(dep:name())
            if table.contains(dep_attrs, "Codegen.Owner") then
                table.insert(codegen_deps, dep:name()..".Codegen")
            end
        end
        return codegen_deps
    end)
analyzer_end()

rule("codegen.fetch")
    on_load(function (target, opt)
        -- config
        local codegen_dir = path.join(target:autogendir({root = true}), target:plat(), "codegen")
        local source_file = path.join(codegen_dir, target:name(), "/generated.cpp")

        -- check generated files
        if not os.exists(source_file) then
            local gen_file = io.open(source_file, "w")
            -- gen_file:print("static_assert(false, \"codegen of module "..target:name().." is not completed!\")")
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
    end)
rule_end()

function codegen_component(owner, opt)
    target(owner)
        add_deps(owner..".Codegen", { public = opt and opt.public or true })
        add_rules("codegen.fetch")
        add_values("Sakura.Attributes", "Codegen.Owner")
    target_end()

    target(owner..".Codegen")
        set_group("01.modules/"..owner.."/codegen")
        set_kind("phony")
        set_policy("build.fence", true)
        add_rules("codegen.headers")
        add_values("Sakura.Attributes", "Analyze.Ignore")
        on_load(function (target)
            target:data_set("meta.owner", owner)
            target:data_set("meta.api", opt.api or target:name():upper())
            if opt and opt.rootdir then
                opt.rootdir = path.absolute(path.join(target:scriptdir(), opt.rootdir))
            end
            target:data_set("meta.rootdir", opt.rootdir)
        end)
end