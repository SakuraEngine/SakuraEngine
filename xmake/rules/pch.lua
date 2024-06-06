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
        if using_msvc then
            buildtarget:add("cxxflags", "-Yu"..path.absolute(header_to_compile), { public = is_shared })
            buildtarget:add("cxxflags", "-FI"..path.absolute(header_to_compile), { public = is_shared })
            buildtarget:add("cxxflags", "-Fp"..path.absolute(pcoutputfile), { public = is_shared })
            buildtarget:add("cxxflags", "-Fo"..path.absolute(pcoutputfile)..".obj", { public = is_shared })
            need_pc_obj = true
        elseif using_clang_cl then
            buildtarget:add("cxxflags", "-I"..path.directory(header_to_compile), { public = is_shared })
            buildtarget:add("cxxflags", "-Yu"..path.filename(header_to_compile), { public = is_shared })
            buildtarget:add("cxxflags", "-FI"..path.filename(header_to_compile), { public = is_shared })
            buildtarget:add("cxxflags", "-Fp"..path.absolute(pcoutputfile), { public = is_shared })
            buildtarget:add("cxxflags", "-Fo"..path.absolute(pcoutputfile)..".obj", { public = is_shared })
            need_pc_obj = true
        elseif using_clang or using_xcode then
            buildtarget:add("cxxflags", "-include", { public = is_shared })
            buildtarget:add("cxxflags", header_to_compile, { public = is_shared })
            buildtarget:add("cxxflags", "-include-pch", { public = is_shared })
            buildtarget:add("cxxflags", pcoutputfile, { public = is_shared })
        else
            raise("PCH: unsupported toolchain!")
        end
        target:data_set("pcoutputfile", pcoutputfile)
        target:data_set("header_to_compile", header_to_compile)
        target:data_set("need_pc_obj", need_pc_obj)
    end)
    before_build(function(target, opt)
        import("core.project.depend")
        import("core.project.project")
        import("core.language.language")
        import("private.action.build.object")

        local buildtarget_name = target:extraconf("rules", "sakura.pcxxheader", "buildtarget")
        local buildtarget = project.target(buildtarget_name)

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
            object.build(buildtarget, sourcebatch, opt)
            -- print(buildtarget:name().." pch compiled! "..header_to_compile)
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
        add_values("Sakura.Attributes", "PCH")
        add_values("Sakura.Attributes", "Analyze.Ignore")
end

------------------------------------PRIVATE PCH------------------------------------

function private_pch(owner_name)
    target(owner_name)
        add_deps(owner_name..".PrivatePCH", {inherit = false})
        add_values("Sakura.Attributes", "PrivatePCH.Owner")
    target_end()

    pch_target(owner_name, owner_name..".PrivatePCH")
        -- private pch generate pch file and inject it to owner
        -- so it is a phony target
        set_kind("phony") 
        add_rules("sakura.pcxxheader", { buildtarget = owner_name, shared = false })
        add_values("Sakura.Attributes", "PrivatePCH")
end

------------------------------------SHARED PCH------------------------------------

analyzer("SharedPCH.Score")
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
analyzer_end()

analyzer("SharedPCH.ShareFrom")
    add_deps("SharedPCH.Score")
    analyze(function(target, attributes, analyzing)
        local share_from = ""
        local has_private_pch = target:values("PrivatePCH.Owner")
        if not has_private_pch then
            local last_score = 0
            for __, dep in pairs(target:deps()) do
                local score = dep:data("SharedPCH.Score") or 0
                if score and score > last_score then
                    last_score = score
                    share_from = dep:name()
                end
            end
        end
        return share_from
    end)
analyzer_end()

function shared_pch(owner_name)
    target(owner_name)
        add_values("Sakura.Attributes", "SharedPCH.Owner")
    target_end()

    pch_target(owner_name, owner_name..".SharedPCH")
        -- public pch generate pch file and links to other targets
        set_kind("phony") 
        add_rules("sakura.pcxxheader", { buildtarget = owner_name..".SharedPCH", shared = true })
        add_deps(owner_name, { inherit = true })
        add_values("Sakura.Attributes", "SharedPCH")

    if (false) then
        target("SharedPCH.Dispatcher")
            add_values("Sakura.Attributes", "SharedPCH.Owners")
        target_end()
    end
end

rule("PickSharedPCH")
    on_load(function(target)
        local tbl_path = "build/.gens/module_infos/"..target:name()..".table"
        if os.exists(tbl_path) then
            local tbl = io.load(tbl_path)
            local share_from = tbl["SharedPCH.ShareFrom"]
            if (share_from ~= "") then
                target:add("deps", share_from..".SharedPCH", { inherit = false })
                print("PickSharedPCH: "..target:name().." pick "..share_from..".SharedPCH")
            end
        end
    end)
rule_end()

----------------------------------------------------------------------------------