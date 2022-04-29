find_sdk = find_sdk or {}

function tooldir()
    return vformat("SDKs/tools/$(host)")
end

function binarydir()
    return vformat("$(buildir)/$(os)/$(arch)/$(mode)")
end

function sdk_from_github(zip)
    import("net.http")
    import("lib.detect.find_file")
    local sdkdir = sdkdir or os.projectdir().."/SDKs"
    local zip_dir = find_file(zip, {sdkdir})
    if(zip_dir == nil) then
        local url = vformat("https://github.com/SakuraEngine/Sakura.Resources/releases/download/SDKs/")
        print("download: "..url..zip.." to: "..os.projectdir().."/SDKs/"..zip)
        http.download(url..zip, os.projectdir().."/SDKs/"..zip, {continue = false})
    end
end

function find_tool_zip(tool_name)
    import("lib.detect.find_file")
    local sdkdir = sdkdir or os.projectdir().."/SDKs"
    local zip_name = vformat(tool_name.."-$(host)-"..os.arch()..".zip")
    local zip_dir = find_file(zip_name, {sdkdir})
    return {name = zip_name, dir = zip_dir}
end

function install_tool(tool_name)
    import("utils.archive")
    import("lib.detect.find_file")

    zip_file = find_tool_zip(tool_name)
    --if(zip_file.dir == nil) then
    --    sdk_from_github(zip_file.name)
    --    zip_file = find_tool_zip(tool_name)
    --end
    
    if(zip_file.dir ~= nil) then
        print("install: "..zip_file.name)
        archive.extract(zip_file.dir, tooldir())
    else
        print("failed to install "..tool_name..", file "..zip_file.name.." not found!")
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

function find_program(name, sdkdir, use_which)
    import("lib.detect.find_file")
    import("lib.detect.find_program")

    local sdkdir = sdkdir or path.join(os.projectdir(), tooldir())
    local prog = find_program(name, {pathes = {sdkdir, "/usr/local/bin"}})
    if(prog == nil) then
        if(os.host() ~= "windows" and use_which ~= nil and use_which == true) then
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