shared_module("SkrRTExporter", "SKR_RUNTIME_EXPORTER", engine_version)
    set_group("01.modules")
    public_dependency("SkrRT", engine_version)
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrRuntimeExporter",
        api = "SKR_RUNTIME_EXPORTER"
    })    
    on_load(function (target, opt)
        local depend = import("core.project.depend")
        local includedir = path.join(os.scriptdir(), "include", "SkrRuntimeExporter", "exporters")
        local runtime = path.join(os.scriptdir(), "..", "runtime", "include")
        -- cgpu/api.h
        local cgpu_header = path.join(runtime, "cgpu", "api.h")
        local cgpu_header2 = path.join(includedir, "cgpu", "api.h")
        depend.on_changed(function ()
            os.vcp(cgpu_header, cgpu_header2)
        end, {dependfile = target:dependfile(cgpu_header), files = {cgpu_header, cgpu_header2, target:targetfile()}})
        target:add("defines",
            "CGPU_EXTERN_C_BEGIN= ", "CGPU_EXTERN_C_END= ",
            "CGPU_EXTERN_C= ", "CGPU_API=sreflect")
    end)
    add_files("src/**.cpp")