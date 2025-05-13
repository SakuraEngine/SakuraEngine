--[[ rule options
{
    scripts = {
        {
            file = "script.py", -- script file
            private = false, -- if true, generated files will not be exported
            import_dirs = { "import/a", "import/b" }, -- script import dirs
        }
    },
    dep_files = { "test/*.py", "test/*.mako" }, -- dep_files
    ...
}
]]
rule("c++.codegen.generators")
    after_load(function (target, opt)
        import("skr.analyze")
        if not analyze.in_analyze_phase() then
            import("skr.codegen")
            codegen.solve_generators(target)
        end
    end)
rule_end()

--[[ rule options
{
    api="XXXX", -- export api name
    rootdir="include/xxxx", -- root dir of generated files
}
-- new options
{
    api="XXXX", -- module api name
    file_groups= {
        {
            files="xxx/*.hpp", -- files to apply codegen
            rootdir="include/xxxx", -- for solve relative path
        },
        ...
    }
}
]]
rule("c++.codegen.meta")
    set_extensions(".h", ".hpp")
    on_prepare(function (proxy_target, opt)
        import("core.project.project")
        import("skr.codegen")
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
    on_prepare(function (proxy_target, opt)
        import("core.project.project")
        import("skr.codegen")
        -- render mako templates
        local owner_name = proxy_target:data("mako.owner")
        local owner = project.target(owner_name)
        codegen.mako_render(owner, opt)
    end)
rule_end()

rule("c++.codegen.load")
    on_load(function (target, opt)
        import("skr.analyze")
        import("skr.utils")

        if xmake.argv()[1] ~= "analyze_project" then
            -- config
            local codegen_dir = path.join(utils.skr_codegen_dir(target:name()), "codegen")
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
            target:add("includedirs", path.join(codegen_dir, target:name()), {public = true})

            -- add deps
            local analyze_tbl = analyze.load(target:name())
            if analyze_tbl then
                local codegen_deps = analyze_tbl["Codegen.Deps"]
                for _, codegen_dep in ipairs(codegen_deps) do
                    target:add("deps", codegen_dep, { public = false })
                end
            end
        end
    end)
rule_end()

analyzer_target("Codegen.Deps")
    analyze(function(target, attributes, analyze_ctx)
        local codegen_deps = {}
        for __, dep in ipairs(target:orderdeps()) do
            local dep_attrs = analyze_ctx.query_attributes(dep:name())
            if table.contains(dep_attrs, "Codegen.Owner") then
                table.insert(codegen_deps, dep:name()..".Mako")
            end
        end
        return codegen_deps
    end)
analyzer_target_end()

analyzer_target("Codegen.MakoDeps")
analyze(function(target, attributes, analyze_ctx)
    local codegen_deps = {}
    for __, dep in ipairs(target:orderdeps()) do
        local dep_attrs = analyze_ctx.query_attributes(dep:name())
        if table.contains(dep_attrs, "Codegen.Owner") then
            table.insert(codegen_deps, dep:name()..".Meta")
        end
    end
    return codegen_deps
end)
analyzer_target_end()