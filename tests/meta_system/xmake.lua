codegen_component("MetaTest", { api = "METATEST", rootdir = "include/MetaTest" })
    add_files("include/**.hpp")

target("MetaTest")
    set_group("05.tests/meta")
    set_kind("binary")
    
    add_includedirs("include")
    add_files("src/**.cpp")
    
    add_deps("SkrBase")

    add_rules("c++.codegen.generators", {
        scripts = {
            -- test
            { 
                file = "meta_scripts/install_test.py", 
                import_dirs={ "meta_scripts/" }, 
                private = true,
            },
        },
        dep_files = {
            "**.py",
            "**.mako"
        }
    })