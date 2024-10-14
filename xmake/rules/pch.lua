rule("sakura.pcxxheader")
    set_extensions(".pch", ".h", ".hpp")
    after_load(function(target, opt)
        import("core.project.project")
        local buildtarget_name = target:extraconf("rules", "sakura.pcxxheader", "buildtarget")
        local is_shared = target:extraconf("rules", "sakura.pcxxheader", "shared")
        local buildtarget = project.target(buildtarget_name)
        local using_msvc = buildtarget:toolchain("msvc")
        local using_clang_cl = buildtarget:toolchain("clang-cl")
        local using_clang = buildtarget:toolchain("clang")
        local using_xcode = buildtarget:toolchain("xcode")

        local header_to_compile = buildtarget:autogenfile(target:name().."_pch.hpp")
        local pcoutputfile = buildtarget:autogenfile(target:name().."_pch.pch")

        local need_pc_obj = false
        local pch_owner_flags = {}
        local pch_flags = {}
        if using_msvc then
            table.insert(pch_owner_flags, "-Yu"..path.absolute(header_to_compile)) --owner
            table.insert(pch_owner_flags, "-FI"..path.absolute(header_to_compile)) --owner
            
            table.insert(pch_flags, "-Fp"..path.absolute(pcoutputfile))      --both
            need_pc_obj = true
        elseif using_clang_cl then
            table.insert(pch_owner_flags, "-Yu"..path.filename(header_to_compile)) --owner
            table.insert(pch_owner_flags, "-FI"..path.filename(header_to_compile)) --owner

            table.insert(pch_flags, "-I"..path.directory(header_to_compile)) --both
            table.insert(pch_flags, "-Fp"..path.absolute(pcoutputfile))      --both
            need_pc_obj = true
        elseif using_clang or using_xcode then
            table.insert(pch_owner_flags, "-include")
            table.insert(pch_owner_flags, header_to_compile) --owner
            table.insert(pch_owner_flags, "-include-pch")
            table.insert(pch_owner_flags, pcoutputfile)      --owner
        else
            raise("PCH: unsupported toolchain!")
        end
        
        buildtarget:add("cxxflags", pch_flags, { public = false })
        buildtarget:add("cxxflags", pch_owner_flags, { public = false })

        target:data_set("pcoutputfile", pcoutputfile)
        target:data_set("header_to_compile", header_to_compile)
        target:data_set("need_pc_obj", need_pc_obj)
        target:data_set("pch_owner_flags", pch_owner_flags)
    end)
    before_build(function(target, opt)
        import("core.project.depend")
        import("core.project.project")
        import("core.language.language")
        import("private.action.build.object")

        -- clone buildtarget
        local buildtarget_name = target:extraconf("rules", "sakura.pcxxheader", "buildtarget")
        local buildtarget = project.target(buildtarget_name)
        local args_target = buildtarget:clone()

        -- remove pch flags when compiling pchs
        local pch_owner_flags = target:data("pch_owner_flags")
        local cxxflags = table.wrap(args_target:get("cxxflags"))
        table.remove_if(cxxflags, function (i, cxxflag)
            return table.contains(pch_owner_flags, cxxflag)
        end)
        args_target:set("cxxflags", cxxflags)

        -- extract files
        local sourcebatches = target:sourcebatches()
        local pchfiles = ""
        if sourcebatches then
            local sourcebatch = sourcebatches["sakura.pcxxheader"]
            assert(#sourcebatch.sourcefiles >= 1)
            pchfiles = sourcebatch.sourcefiles
        end

        -- generate proxy header
        local header_to_compile = target:data("header_to_compile")
        depend.on_changed(function ()
            local include_content = ""
            for _, pchfile in pairs(pchfiles) do
                include_content = include_content .. "#include \"" .. path.absolute(pchfile):gsub("\\", "/") .. "\"\n"
            end

            io.writefile(header_to_compile, ([[
#pragma system_header
#ifdef __cplusplus
#ifdef _WIN32
#include <intrin.h>
#endif
%s
#endif // __cplusplus
            ]]):format(include_content))
        end, {dependfile = target:dependfile(target:name()..".sakura.pch"), files = pchfiles})

        -- build pch
        local pcoutputfile = target:data("pcoutputfile")
        if header_to_compile then
            local sourcefile = header_to_compile
            local dependfile = buildtarget:dependfile(pcoutputfile)
            local sourcekind = language.langkinds()["cxx"]
            local sourcebatch = {sourcekind = sourcekind, sourcefiles = {sourcefile}, objectfiles = {pcoutputfile}, dependfiles = {dependfile}}
            object.build(args_target, sourcebatch, opt)
        end

        -- insert pc objects
        local need_pc_obj = target:data("need_pc_obj")
        if need_pc_obj then
            local objectfiles = buildtarget:objectfiles()
            if objectfiles then
                table.insert(objectfiles, path.absolute(pcoutputfile) .. ".obj")
            end
        end
    end)
rule_end()

function pch_target(owner_name, pch_target_name)
    target(pch_target_name)
        set_group("01.modules/"..owner_name.."/components")
        set_policy("build.fence", true)
        -- temporaly close the exception for pch target
        set_exceptions("no-cxx")
        analyzer_attribute("PCH")
        analyzer_ignore()
end

------------------------------------PRIVATE PCH------------------------------------

function private_pch(owner_name)
    target(owner_name)
        add_deps(owner_name..".PrivatePCH", {inherit = false})
        analyzer_attribute("PrivatePCH.Owner")
    target_end()

    pch_target(owner_name, owner_name..".PrivatePCH")
        -- private pch generate pch file and inject it to owner
        set_kind("headeronly")
        analyzer_attribute("PrivatePCH")
        add_rules("sakura.pcxxheader", { buildtarget = owner_name, shared = false })
        add_rules("sakura.derived_target", { owner_name = owner_name })
end

------------------------------------SHARED PCH------------------------------------

analyzer_target("SharedPCH.Score")
    analyze(function(target, attributes, analyzing)
        if not table.contains(attributes, "SharedPCH.Owner") then
            return 0
        end

        local score = 1
        for __, dep in pairs(target:deps()) do
            local dep_attrs = analyzing.query_attributes(dep:name())
            if table.contains(dep_attrs, "SharedPCH.Owner") then
                score = score + 1
            end
        end
        target:data_set("SharedPCH.Score", score)
        return score
    end)
analyzer_target_end()

analyzer_target("SharedPCH.ShareFrom")
    add_deps("__Analyzer.SharedPCH.Score", { order = true })
    analyze(function(target, attributes, analyzing)
        local share_from = ""
        local has_private_pch = table.contains(attributes, "PrivatePCH.Owner")
        if not has_private_pch then
            local last_score = 1
            local last_subscore = 0
            local score_table = {}
            for __, dep in ipairs(target:orderdeps()) do
                local score = dep:data("SharedPCH.Score") or 0
                local subscore = #dep:orderdeps() * #(dep:get("files") or {}) or -1
                local cond0 = score > 0 and score > last_score
                local cond1 = score > 0 and score == last_score and subscore > last_subscore
                if cond0 or cond1 then
                    last_score = score
                    last_subscore = subscore
                    share_from = dep:name()
                end
                score_table[dep:name()] = { score, subscore }
            end
            -- check collides
            for name, scores in pairs(score_table) do
                if name ~= share_from and scores[1] == last_score and scores[2] == last_subscore then
                    print(score_table)
                    print(name)
                    print(share_from)
                    raise("SharedPCH: collides between pickable SharedPCH targets!")
                end
            end
        end
        return share_from
    end)
analyzer_target_end()

function shared_pch(owner_name)
    target(owner_name)
        analyzer_attribute("SharedPCH.Owner")
    target_end()

    pch_target(owner_name, owner_name..".SharedPCH")
        -- shared pch generate pch/obj file and link them to others
        set_kind("object") 
        add_rules("sakura.pcxxheader", { buildtarget = owner_name..".SharedPCH", shared = true })
        add_deps(owner_name)
        analyzer_attribute("SharedPCH")
        add_rules("sakura.derived_target", { owner_name = owner_name })
end

rule("PickSharedPCH")
    on_load(function(target)
        if xmake.argv()[1] ~= "analyze_project" then
            local tbl_path = "build/.gens/module_infos/"..target:name()..".table"
            if os.exists(tbl_path) then
                local tbl = io.load(tbl_path)
                local share_from = tbl["SharedPCH.ShareFrom"]
                if (share_from ~= "") then
                    target:add("deps", share_from..".SharedPCH", { inherit = false })
                    target:data_set("SharedPCH.ShareFrom", share_from..".SharedPCH")
                end
            end
        end
    end)
    before_build(function(target)
        import("core.project.project")
        local share_from = target:data("SharedPCH.ShareFrom")
        if share_from then
            local pch_target = project.target(share_from)
            -- add cxxflags
            target:add("cxxflags", pch_target:get("cxxflags"), { public = false })
            -- symbols
            local pch_symbols = pch_target:get("symbols") or "none"
            target:set("symbols", pch_symbols)
            -- copy pdb
            local using_msvc = target:toolchain("msvc")
            if using_msvc and pch_target then
                local pch_pdb = "build/$(os)/$(arch)/$(mode)/compile."..pch_target:name()..".pdb"
                if os.exists(pch_pdb) then
                    os.cp(pch_pdb, "build/$(os)/$(arch)/$(mode)/compile."..target:name()..".pdb")
                end
            end
        end
    end)
rule_end()

----------------------------------------------------------------------------------