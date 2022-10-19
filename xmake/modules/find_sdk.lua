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

    local zip_file = find_tool_zip(tool_name)

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
    if(is_mode("debug") or is_mode("releasedbg")) then
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

function Split(s, delimiter)
    local result = {};
    for match in (s..delimiter):gmatch("(.-)"..delimiter) do
        table.insert(result, match);
    end
    return result;
end

function find_program(name, sdkdir, use_which, force_in_sdkdir)
    local detect = import("lib.detect")
    local sdkdir = sdkdir or path.join(os.projectdir(), tooldir())

    -- find in project embed tool dir
    local prog = detect.find_program(name, {paths = {sdkdir}}) 
    local vexec = nil

    if (not force_in_sdkdir) then
        -- use xmake find_tool / find_program in path
        local paths = nil
        if(os.host() == "windows") then
            paths = Split(os.getenv("PATH"), ";")
        end
        if(prog == nil) then
            local tool = tool or detect.find_tool(name, {paths = {sdkdir, "/usr/local/bin", paths}})
            prog = tool and tool.program or detect.find_program(name, {paths = {sdkdir, "/usr/local/bin"}})
        end

        -- use which
        if(prog == nil) then
            if(os.host() ~= "windows" and use_which ~= nil and use_which == true) then
                local outdata, errdata = os.iorun("which "..name)
                if(errdata~= nil or errdata ~= "") then
                    prog = string.gsub(outdata, "%s+", "")
                end
            end
        end
    end

    -- find with find_file
    if(prog == nil) then
        vprint(name.." not found! under "..sdkdir)
        local progf = detect.find_file(name, {sdkdir})
        if(os.host() == "windows") then
            if(progf == nil) then
                progf = detect.find_file(name..".exe", {sdkdir})
            end
        end
        if(progf == nil) then
            vprint(name.."_f not found! under "..sdkdir)
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

function find_embed_python()
    local embed = find_program("python", path.join(os.projectdir(), tooldir(), "python-embed"), false, true)
    import("core.base.option")
    local verbose = option.get("verbose")
    if verbose then
        print("found embed python:")
        print(embed)
    end
    return embed
end