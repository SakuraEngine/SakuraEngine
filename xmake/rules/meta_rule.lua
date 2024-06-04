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
    before_buildcmd_files(function (target, batchcmds, sourcebatch, opt)
        import("core.project.project")
        import("codegen")

        local owner_name = target:data("meta.owner")
        local owner = project.target(owner_name)
        local files = sourcebatch.sourcefiles

        codegen.collect_headers_batch(owner, files)
    end)
    on_buildcmd_files(function (proxy_target, batchcmds, sourcebatch, opt)
        import("core.project.project")
        import("codegen")

        local owner_name = proxy_target:data("meta.owner")
        local owner = project.target(owner_name)
        local files = sourcebatch.sourcefiles
        local abs_out = sourcebatch.metadir

        -- compile meta file
        local meta_target = owner:clone()
        meta_target:set("pcxxheader", nil)
        meta_target:set("pcheader", nil)
        codegen.meta_compile(meta_target, proxy_target, batchcmds, opt)

        -- render mako templates
        codegen.mako_render(meta_target, proxy_target, batchcmds, opt)
    end)

function codegen_component(owner, opt)
    target(owner)
        add_deps(owner..".Codegen", { public = opt and opt.public or true })
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
        end)
    target_end()

    target(owner..".Codegen")
        set_group("01.modules/"..owner.."/codegen")
        set_kind("static")
        set_policy("build.fence", true)
        add_rules("codegen.headers")
        on_load(function (target)
            target:data_set("meta.owner", owner)
            target:data_set("meta.api", opt.api or target:name():upper())
            if opt and opt.rootdir then
                opt.rootdir = path.absolute(path.join(target:scriptdir(), opt.rootdir))
            end
            target:data_set("meta.rootdir", opt.rootdir)
        end)
end