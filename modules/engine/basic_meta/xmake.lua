target("SkrBasicMeta")
    set_kind("headeronly")
    add_rules("c++.codegen.generators", {
        scripts = {
            -- baisc
            { file = "basic/basic.py", use_new_framework = true },
            -- ecs
            -- { file = "ecs/component.py" },
            { file = "ecs/ecs.py", use_new_framework = true },
            -- rttr
            { file = "rttr/rttr.py", use_new_framework = true },
            -- serialize
            { file = "serialize/serialize.py" },
            { file = "serialize/serialize_json.py" },
            -- trait object
            { file = "trait_object/trait_object.py" },
        }, 
        dep_files = {
            "**.py",
            "**.mako"
        }
    })

target("SkrRT")
    add_deps("SkrBasicMeta")