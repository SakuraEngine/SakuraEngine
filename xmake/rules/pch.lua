rule("sakura.pcxxheader")
    set_extensions(".pch", ".h", ".hpp")
    before_build(function(target, opt)
        import("core.project.depend")
        import("core.project.project")
        import("core.language.language")
        import("private.action.build.object")

        local buildtarget_name = target:extraconf("rules", "sakura.pcxxheader", "buildtarget")
        local is_shared = target:extraconf("rules", "sakura.pcxxheader", "shared")
        local buildtarget = project.target(buildtarget_name)
        local using_msvc = buildtarget:toolchain("msvc")
        local using_clang_cl = buildtarget:toolchain("clang-cl")
        local using_clang = buildtarget:toolchain("clang")
        local using_xcode = buildtarget:toolchain("xcode")

        -- extract files
        local sourcebatches = target:sourcebatches()
        local pchfiles = ""
        if sourcebatches then
            local sourcebatch = sourcebatches["sakura.pcxxheader"]
            assert(#sourcebatch.sourcefiles >= 1)
            pchfiles = sourcebatch.sourcefiles
        end

        -- generate proxy header
        local header_to_compile = buildtarget:autogenfile(target:name().."_pch.hpp")
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
        local pcoutputfile = ""
        if header_to_compile then
            local sourcefile = header_to_compile
            pcoutputfile = buildtarget:autogenfile(target:name().."_pch.pch")
            local dependfile = buildtarget:dependfile(pcoutputfile)
            local sourcekind = language.langkinds()["cxx"]
            local sourcebatch = {sourcekind = sourcekind, sourcefiles = {sourcefile}, objectfiles = {pcoutputfile}, dependfiles = {dependfile}}
            object.build(buildtarget, sourcebatch, opt)
        end

        -- insert to owner
        local need_pc_obj = false
        if using_msvc then
            buildtarget:add("cxxflags", "-Yu"..path.absolute(header_to_compile), { public = is_shared })
            buildtarget:add("cxxflags", "-FI"..path.absolute(header_to_compile), { public = is_shared })
            buildtarget:add("cxxflags", "-Fp"..path.absolute(pcoutputfile), { public = is_shared })
            need_pc_obj = true
        elseif using_clang_cl then
            buildtarget:add("cxxflags", "-I"..path.directory(header_to_compile), { public = is_shared })
            buildtarget:add("cxxflags", "-Yu"..path.filename(header_to_compile), { public = is_shared })
            buildtarget:add("cxxflags", "-FI"..path.filename(header_to_compile), { public = is_shared })
            buildtarget:add("cxxflags", "-Fp"..path.absolute(pcoutputfile), { public = is_shared })
            need_pc_obj = true
        elseif using_clang or using_xcode then
            buildtarget:add("cxxflags", "-include", { public = is_shared })
            buildtarget:add("cxxflags", header_to_compile, { public = is_shared })
            buildtarget:add("cxxflags", "-include-pch", { public = is_shared })
            buildtarget:add("cxxflags", pcoutputfile, { public = is_shared })
        else
            raise("PCH: unsupported toolchain!")
        end

        -- insert pc objects
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
end

------------------------------------PRIVATE PCH------------------------------------

function private_pch(owner_name)
    target(owner_name)
        add_deps(owner_name..".PrivatePCH", {public = false})
    target_end()

    pch_target(owner_name, owner_name..".PrivatePCH")
        -- private pch generate pch file and inject it to owner
        -- so it is a phony target
        set_kind("phony") 
        add_rules("sakura.pcxxheader", { buildtarget = owner_name, shared = false })
end

------------------------------------SHARED PCH------------------------------------

target("SharedPCH.Dispatcher")
    set_kind("phony")
    set_group("00.utilities")
    set_policy("build.fence", true)
    on_build(function(target, opt)
        import("core.project.project")

        -- score owners
        local owner_names = target:values("SharedPCH.Owners")
        local owner_scores = {}
        for _, owner_name in pairs(owner_names) do
            owner_scores[owner_name] = owner_scores[owner_name] or 1
            for _, owner_dep in pairs(project.target(owner_name):deps()) do
                if table.contains(owner_names, owner_dep:name()) then
                    owner_scores[owner_name] = owner_scores[owner_name] + 1
                end
            end
        end

        -- print owner scores
        for owner_name, score in pairs(owner_scores) do
            print("owner: " .. owner_name .. ", score: " .. score)
        end

        -- dispatch to downstream targets without private pch modules
        for _, target in pairs(project.ordertargets()) do
            
        end
    end)
target_end()

function shared_pch(owner_name)
    target("SharedPCH.Dispatcher")
        add_values("SharedPCH.Owners", owner_name)
    target_end()

    target(owner_name)
        add_deps("SharedPCH.Dispatcher", { public = true })
    target_end()

    pch_target(owner_name, owner_name..".SharedPCH")
        -- public pch generate pch file and links to other targets
        -- so it is a static target
        set_kind("static") 
        add_rules("sakura.pcxxheader", { buildtarget = owner_name..".SharedPCH", shared = true })
        add_deps(owner_name, {public = false})
end

----------------------------------------------------------------------------------