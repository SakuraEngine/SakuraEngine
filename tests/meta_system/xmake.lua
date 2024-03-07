target("MetaTest")
    set_group("05.tests/meta")
    set_kind("binary")
    
    add_includedirs("include")
    add_files("src/**.cpp")
    
    add_deps("SkrBase")
    
    -- add_rules("c++.codegen", {
    --     files = {"include/**.h", "include/**.hpp"},
    --     rootdir = "include/MetaTest/",
    --     use_new_framework = true,
    -- })

    -- add_rules("c++.meta.generators", {
    --     scripts = {
    --         { file = "meta_scripts/test.py" },
    --     },
    --     dep_files = {
    --         "**.py",
    --         "**.mako"
    --     }
    -- })