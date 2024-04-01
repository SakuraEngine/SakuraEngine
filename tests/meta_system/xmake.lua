target("MetaTest")
    set_group("05.tests/meta")
    set_kind("binary")
    
    add_includedirs("include")
    add_files("src/**.cpp")
    
    add_deps("SkrBase")
    
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/MetaTest/",
    })

    add_rules("c++.meta.generators", {
        scripts = {
            -- test
            { 
                file = "meta_scripts/install_test.py", 
                import_dirs={ "meta_scripts/" }, 
                private = true,
                use_new_framework = true,
            },
        },
        dep_files = {
            "**.py",
            "**.mako"
        }
    })