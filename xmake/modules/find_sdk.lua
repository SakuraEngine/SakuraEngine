find_sdk = find_sdk or {}

function tooldir()
    return vformat("SDKs/tools/$(host)")
end

function binarydir()
    return vformat("$(buildir)/$(os)/$(arch)/$(mode)")
end

function install_tool(tool_name)
    import("utils.archive")
    import("lib.detect.find_file")

    local sdkdir = sdkdir or os.projectdir().."/SDKs"
    local zip_file = vformat(tool_name.."-$(host)-"..os.arch()..".zip")
    print("install: "..zip_file)
    local zip_dir = find_file(zip_file, {sdkdir})
    if(zip_dir ~= nil) then
        archive.extract(zip_dir, tooldir())
    else
        print("failed to install "..tool_name..", file "..zip_file.." not found!")
    end
end

function install_lib(lib_name)
    import("utils.archive")
    import("lib.detect.find_file")
    import("core.project.config")

    local sdkdir = sdkdir or os.projectdir().."/SDKs"
    local zip_file = vformat(lib_name.."-$(os)-"..config.arch()..".zip")
    local zip_dir = nil
    print("install: "..zip_file)
    if(is_mode("debug")) then
        local zip_file_d = vformat(lib_name.."_d-$(os)-"..config.arch()..".zip")
        zip_dir = find_file(zip_file_d, {sdkdir})
    end
    if(zip_dir == nil) then
        zip_dir = find_file(zip_file, {sdkdir})
    end
    if(zip_dir ~= nil) then
        archive.extract(zip_dir, binarydir())
    else
        print("failed to install "..lib_name..", file "..zip_file.." not found!")
    end
end

function find_program(name, sdkdir)
    import("lib.detect.find_file")
    import("lib.detect.find_program")

    local sdkdir = sdkdir or path.join(os.projectdir(), tooldir())
    local prog = find_program(name, {pathes = {sdkdir, "/usr/local/bin"}})
    if(prog == nil) then
        if(os.host() ~= "windows") then
            local outdata, errdata = os.iorun("which "..name)
            if(errdata ~= "") then
                prog = string.gsub(outdata, "%s+", "")
            end
        end
    end
    
    if(prog == nil) then
        print(name.." not found! under "..sdkdir)
        local progf = find_file(name, {sdkdir})
        if(os.host() == "windows") then
            if(progf == nil) then
                progf = find_file(name..".exe", {sdkdir})
            end
        end
        if(progf == nil) then
            print(name.."_f not found! under "..sdkdir)
            return
        else
            prog = progf
            vexec = "cd "..sdkdir.." && "..prog
        end
    else
        vexec = prog
    end
    return {program = prog, vexec = vexec}
end