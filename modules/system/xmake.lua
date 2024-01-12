target("SkrSystem")
    set_group("01.modules")
    set_kind("shared")
    -- add source files
    add_files("src/build.*.cpp")
    add_deps("SkrMemory", "SkrProfile", {public = true, inherit = true})
    add_includedirs("include", {public = true})