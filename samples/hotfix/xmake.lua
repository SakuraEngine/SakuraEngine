shared_module("HotfixTest", "HOTFIX_TEST", engine_version)
    set_group("04.examples/hotfix")
    public_dependency("SkrRT", engine_version)
    add_files("hotfix_module.cpp")
    add_rules("c++.codegen", {
        files = {"*.h", "*.hpp"},
        rootdir = "./",
        api = "HOTFIX_TEST"
    })

target("HotfixTestHost")
    set_group("04.examples/hotfix")
    set_kind("binary")
    add_deps("SkrRT", {inherit = true})
    add_deps("HotfixTest", {inherit = false})
    on_config(function (target)
        local dep = target:dep("HotfixTest")
        local includes = dep:get("includedirs", {interface=true})
        for _, include in ipairs(includes) do
            target:add("includedirs", include)
        end
    end)
    add_files("main.cpp")