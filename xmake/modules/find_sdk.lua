find_sdk = find_sdk or {}

function find_program(name)
    import("lib.detect.find_file")
    import("lib.detect.find_program")
    local sdkdir = path.join(os.projectdir(), "build/sdk")
    local dxc = find_program(name, {pathes = {sdkdir}})
    local dxcf = find_file(name, {sdkdir})
    
    if(dxc == nil) then
        print(name.." not found! under "..sdkdir)
        if(dxcf == nil) then
            print(name.."_f not found! under "..sdkdir)
            return
        else
            dxc = dxcf
            vexec = "cd "..sdkdir.." && "..dxc
        end
    else
        vexec = dxc
    end
    return {program = dxc, vexec = vexec}
end