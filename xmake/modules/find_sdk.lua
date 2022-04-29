find_sdk = find_sdk or {}

function find_program(name, sdkdir)
    import("lib.detect.find_file")
    import("lib.detect.find_program")
    local sdkdir = sdkdir or path.join(os.projectdir(), "build/sdk")
    local prog = find_program(name, {pathes = {sdkdir, "/usr/local/bin"}})
    if(prog == nil) then
        local outdata, errdata = os.iorun("which grpc_cpp_plugin")
        prog = string.gsub(outdata, "%s+", "")
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