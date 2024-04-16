-- no export api here, static_module may better
shared_module("SkrScript", "SKR_SCRIPT_STATIC", engine_version)
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_rules("c++.codegen", {
            files = {"include/**.h", "include/**.hpp"},
            rootdir = "include/SkrScript",
            api = "SKR_SCRIPT"
        })
    add_rules("c++.meta.generators", {
        scripts = {
            {file = "codegen/function_bind.py", use_new_framework=true, private=true}
        },
        dep_files = {
            "codegen/**.py",
            "codegen/**.mako"
        }
    })