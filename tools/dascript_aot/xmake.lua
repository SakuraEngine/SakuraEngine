shared_module("SkrDAScriptAOTPlugin", "SKR_DASCRIPT_AOT_PLUGIN", engine_version)
    set_group("02.tools")
    add_includedirs("aot_plugin/include", {public=true})
    add_includedirs("aot_plugin/src", {public=false})
    public_dependency("SkrToolCore", engine_version)
    add_files("aot_plugin/src/**.cpp")
    -- TODO: remove this dependency
    add_packages("daScript", { public = false })


executable_module("SkrDAScriptAOTCompiler", "SKR_RESOURCE_COMPILER", engine_version)
    set_group("02.tools")
    add_includedirs("aot_cc/src", {public=false})
    public_dependency("SkrDAScript", engine_version)
    public_dependency("SkrDAScriptAOTPlugin", engine_version)
    add_files("aot_cc/src/**.cpp")
    -- TODO: remove this dependency
    add_packages("daScript", { public = false })