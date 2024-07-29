target("SkrBasicMeta")
    set_kind("headeronly")
    add_rules("c++.codegen.generators", {
        scripts = {
            -- baisc
            { file = "basic/basic.py" },
            -- ecs
            { file = "ecs/ecs.py" },
            -- rttr
            { file = "rttr/rttr.py" },
            -- serialize
            { file = "serialize/serialize.py" },
            -- proxy
            { file = "proxy/proxy.py" },
        }, 
        dep_files = {
            "**.py",
            "**.mako"
        }
    })

target("SkrRT")
    add_deps("SkrBasicMeta")