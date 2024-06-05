function pch_target_name(owner_name)
    return owner_name..".PCH"
end

function pch_target(owner_name)
    target(pch_target_name(owner_name))
        set_kind("phony")
        set_group("01.modules/"..owner_name.."/components")
        set_policy("build.fence", true)
        add_rules("sakura.pcxxheader", { owner_name = owner_name })
end

function private_pch(owner_name)
    target(owner_name)
        add_deps(pch_target_name(owner_name), {public = false})
    target_end()

    pch_target(owner_name)
end

function shared_pch(owner_name)
    target(owner_name)
        add_deps(pch_target_name(owner_name), {public = false})
    target_end()

    pch_target(owner_name)
end

rule("sakura.pcxxheader")
    set_extensions(".pch", ".h", ".hpp")
    before_build(function(target, opt)
        import("core.project.project")
        import("core.language.language")
        import("private.action.build.object")

        local owner_name = target:extraconf("rules", "sakura.pcxxheader", "owner_name")
        local owner = project.target(owner_name)
        local using_msvc = owner:toolchain("msvc")
        local using_clang_cl = owner:toolchain("clang-cl")
        local using_clang = owner:toolchain("clang")
        local using_xcode = owner:toolchain("xcode")

        -- extract files
        local sourcebatches = target:sourcebatches()
        local pchfile = ""
        if sourcebatches then
            local sourcebatch = sourcebatches["sakura.pcxxheader"]
            assert(#sourcebatch.sourcefiles == 1)
            pchfile = sourcebatch.sourcefiles[1]
        end

        -- generate proxy header
        local header_to_compile = nil
        if using_msvc or using_clang_cl then
            header_to_compile = owner:autogenfile(pchfile..".hpp")
            if not os.isfile(header_to_compile) then
                io.writefile(header_to_compile, ([[
#pragma system_header
#ifdef __cplusplus
#include "%s"
#endif // __cplusplus
                ]]):format(path.absolute(pchfile):gsub("\\", "/")))
            end
        else
            header_to_compile = pchfile
        end

        -- build pch
        local pcoutputfile = ""
        if header_to_compile then
            local sourcefile = header_to_compile
            pcoutputfile = owner:autogenfile(pchfile..".pch")
            local dependfile = owner:dependfile(pcoutputfile)
            local sourcekind = language.langkinds()["cxx"]
            local sourcebatch = {sourcekind = sourcekind, sourcefiles = {sourcefile}, objectfiles = {pcoutputfile}, dependfiles = {dependfile}}
            object.build(owner, sourcebatch, opt)
        end

        -- insert to owner
        local need_pc_obj = false
        if using_msvc then
            owner:add("cxxflags", "-Yu"..path.absolute(header_to_compile))
            owner:add("cxxflags", "-FI"..path.absolute(header_to_compile))
            owner:add("cxxflags", "-Fp"..path.absolute(pcoutputfile))
            need_pc_obj = true
        elseif using_clang_cl then
            owner:add("cxxflags", "-I"..path.directory(header_to_compile))
            owner:add("cxxflags", "-Yu"..path.filename(header_to_compile))
            owner:add("cxxflags", "-FI"..path.filename(header_to_compile))
            owner:add("cxxflags", "-Fp"..path.absolute(pcoutputfile))
            need_pc_obj = true
        elseif using_clang or using_xcode then
            owner:add("cxxflags", "-include")
            owner:add("cxxflags", header_to_compile)
            owner:add("cxxflags", "-include-pch")
            owner:add("cxxflags", pcoutputfile)
        else
            raise("PCH: unsupported toolchain!")
        end

        -- insert pc objects
        if need_pc_obj then
            local objectfiles = owner:objectfiles()
            if objectfiles then
                table.insert(objectfiles, path.absolute(pcoutputfile) .. ".obj")
            end
        end
    end)