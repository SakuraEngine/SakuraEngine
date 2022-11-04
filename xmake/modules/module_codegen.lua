import("core.base.option")
import("core.project.project")
import("core.project.depend")

module_codegen = module_codegen or {}

function resolve_skr_module_dependencies(target)
    local pub_deps = target:values("skr.module.public_dependencies")
    local calculated_flatten = {}
    for _, dep in ipairs(pub_deps) do
        local dep_target = project.target(dep)
        if dep_target:rule("skr.module") then
            calculated_flatten[dep] = dep_target
        end
        for _, depdep in ipairs(dep_target:orderdeps()) do
            if depdep:rule("skr.module") then
                calculated_flatten[depdep:name()] = depdep
            end
        end
    end
    local dep_modules = {}
    for _, dep in ipairs(target:orderdeps()) do
        if dep:rule("skr.module") then
            table.insert(dep_modules, dep:name())
            assert(calculated_flatten[dep:name()], 
                "linked skr.module "..dep:name().." is not public dependency of "..target:name()..", please add it to public/private_dependencies")
        end
    end
    return dep_modules
end

function skr_module_gen_json(target, filename, dep_modules)
    -- need build this target?
    local last = os.time()
    local dependfile = target:dependfile(filename)
    local dependinfo = depend.load(dependfile) or {}
    if not depend.is_changed(dependinfo, {lastmtime = os.mtime(filename), values = dep_modules, files = {filename}}) then
        return
    end
    -- start rebuild json
    -- print("generate target metadata: "..target:name())
    local self_version = target:values("skr.module.version")
    local pub_deps = target:values("skr.module.public_dependencies")
    assert(self_version, "module version not found: "..target:name())
    local json_content = "{\n\"module\": {\n"
    json_content = json_content.."\t\"name\": \"" .. target:name().."\",\n"
    json_content = json_content.."\t\"prettyname\": \"" .. target:name().."\",\n"
    json_content = json_content.."\t\"version\": \""..self_version.."\",\n"
    json_content = json_content.."\t\"linking\": \""..target:kind().."\",\n"
    json_content = json_content.."\t\"author\": \"unknown\",\n"
    json_content = json_content.."\t\"url\": \"https://github.com/SakuraEngine/SakuraEngine\",\n"
    json_content = json_content.."\t\"license\": \"EngineDefault (MIT)\",\n"
    json_content = json_content.."\t\"copyright\": \"EngineDefault\",\n"
    -- dep body
    json_content = json_content.."\t\"dependencies\": [\n "
    for _, dep in pairs(pub_deps) do
        local deptarget = project.target(dep)
        assert(deptarget:rule("skr.module"), "public dependency must be a skr.module: "..deptarget:name())
        local depversion = target:values(dep..".version")
        json_content = json_content.."\t\t{ \"name\":\""..dep.."\", \"version\": \""..depversion.."\" },\n"
    end
    json_content = json_content.sub(json_content, 1, -3)
    json_content = json_content.."\n\t]\n}\n}"
    -- end dep body
    -- write json & update files and values to the dependent file
    io.writefile(filename,json_content)
    dependinfo.files = {filename}
    dependinfo.values = dep_modules
    depend.save(dependinfo, dependfile)
    -- output time
    if not option.get("quiet") then
        local now = os.time()
        cprint("${green}[%s]: skr.module.exportjson ${clear}%s cost ${red}%d seconds", target:name(), path.relative(filename), now - last)
    end
end

function skr_module_gen_cpp(target, filename, dep_modules)
    -- need build this target?
    local last = os.time()
    local dependfile = target:dependfile(filename)
    local dependinfo = depend.load(dependfile) or {}
    if not depend.is_changed(dependinfo, {lastmtime = os.mtime(filename), values = dep_modules, files = {filename}}) then
        return
    end
    -- start rebuild json
    -- print("generate target metadata: "..target:name())
    local self_version = target:values("skr.module.version")
    local pub_deps = target:values("skr.module.public_dependencies")
    assert(self_version, "module version not found: "..target:name())
    local delim = "__de___l___im__("
    local delim2 = ")__de___l___im__"
    local cpp_content = "#include \"module/module.hpp\"\n\nSKR_MODULE_METADATA(u8R\""..delim.."\n{\n"
    cpp_content = cpp_content.."\t\"api\": \"" .. self_version.."\",\n"
    cpp_content = cpp_content.."\t\"name\": \"" .. target:name() .."\",\n"
    cpp_content = cpp_content.."\t\"prettyname\": \"" .. target:name() .."\",\n"
    cpp_content = cpp_content.."\t\"version\": \"".. self_version .."\",\n"
    cpp_content = cpp_content.."\t\"linking\": \"".. target:kind() .."\",\n"
    cpp_content = cpp_content.."\t\"author\": \"unknown\",\n"
    cpp_content = cpp_content.."\t\"url\": \"https://github.com/SakuraEngine/SakuraEngine\",\n"
    cpp_content = cpp_content.."\t\"license\": \"EngineDefault (MIT)\",\n"
    cpp_content = cpp_content.."\t\"copyright\": \"EngineDefault\",\n"
    -- dep body
    cpp_content = cpp_content.."\t\"dependencies\": [\n "
    for _, dep in pairs(pub_deps) do
        local deptarget = project.target(dep)
        assert(deptarget:rule("skr.module"), "public dependency must be a skr.module: "..deptarget:name())
        local depversion = target:values(dep..".version")
        cpp_content = cpp_content.."\t\t{ \"name\":\""..dep.."\", \"version\": \""..depversion.."\" },\n"
    end
    cpp_content = cpp_content.sub(cpp_content, 1, -3)
    cpp_content = cpp_content.."\n\t]\n}\n"..delim2.."\", "..target:name()..")"
    -- end dep body
    -- write json & update files and values to the dependent file
    io.writefile(filename, cpp_content)
    dependinfo.files = { filename }
    dependinfo.values = dep_modules
    depend.save(dependinfo, dependfile)
    -- output time
    if not option.get("quiet") then
        local now = os.time()
        cprint("${green}[%s]: skr.module.exportcpp ${clear}%s cost ${red}%d seconds", target:name(), path.relative(filename), now - last)
    end
end