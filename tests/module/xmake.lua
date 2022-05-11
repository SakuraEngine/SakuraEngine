target("static0")
    set_kind("static")
    add_deps("SkrRT")
    set_languages("c++17")
    add_files("test/static0.cpp")

target("dynamic0")
    set_kind("shared")
    add_deps("SkrRT")
    set_languages("c++17")
    add_files("test/dynamic0.cpp")

target("dynamic1")
    set_kind("shared")
    add_deps("SkrRT")
    set_languages("c++17")
    add_files("test/dynamic1.cpp")

target("module-test")
    set_kind("binary")
    add_deps("SkrRT", "static0") -- static modules must be explicit linked
    -- actually there is no need to strongly link these dynamic modules
    add_deps("dynamic0", "dynamic1") 
    add_packages("gtest")
    add_files("test/main.cpp")
    set_languages("c++17")