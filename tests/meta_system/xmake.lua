target("MetaTest")
    set_group("05.tests/meta")
    set_kind("binary")
    
    add_includedirs("include")
    add_files("src/**.cpp")
    
    add_deps("SkrBase")
    
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/MetaTest/",
        use_new_framework = true,
    })

    add_rules("c++.meta.generators", {
        scripts = {
            -- test
            { file = "meta_scripts/test_script/install_test.py", import_dirs={"meta_scripts/test_script/"}, private = true },
            -- gen script
            { file = "meta_scripts/test_gen_scritp/basic/basic.py" },
            { file = "meta_scripts/test_gen_scritp/guid/guid.py" },
            { file = "meta_scripts/test_gen_scritp/rttr/rttr.py" },
        },
        dep_files = {
            "**.py",
            "**.mako"
        }
    })