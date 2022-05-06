target("fs-test")
    set_kind("binary")
    add_deps("SkrRT")
    add_packages("gtest")
    add_files("test/main.cpp")
    set_languages("c++17")
    -- app
    add_rules("xcode.application")
    add_files(os.projectdir().."/cmake/Info.plist")