add_requires("daScript 2023.4.26-skr.1")

includes("rules/AOT.lua")

target("daScriptVSCode")
    set_group("06.devs")
    set_kind("phony")
    add_packages("daScript", { public = false })
    add_rules("@daScript/Standard", { 
        outdir = path.absolute(os.projectdir().."/.vscode") 
    })

shared_module("SkrDAScript", "SKR_DASCRIPT", engine_version)
    add_deps("daScriptVSCode")
    set_exceptions("no-cxx")
    set_optimize("fastest")
    add_packages("daScript", { public = false })
    add_rules("@daScript/Standard", { outdir = "." })
    add_includedirs("daScript/include", { public=true })
    add_files("daScript/src/**.cpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    public_dependency("SkrRTExporter", engine_version)