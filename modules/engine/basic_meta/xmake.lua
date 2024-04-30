target("SkrBasicMeta")
    set_kind("headeronly")
    add_rules("c++.meta.generators", {
        scripts = {
            -- baisc
            { file = "basic/basic.py", use_new_framework = true },
            -- ecs
            { file = "ecs/component.py" },
            { file = "ecs/query.py"},
            -- lua
            -- { file = "lua/luabind.py" }, -- FIXME. lua support
            -- rttr
            { file = "rttr/rttr.py" },
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