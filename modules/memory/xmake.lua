target("SkrMemory")
    set_group("01.modules")
    set_kind("shared")
    -- add source files
    add_files("src/build.*.c")
    add_deps("SkrBase", "SkrProfile", {public = true, inherit = true})
    add_deps("mimalloc", {public = false})
    add_includedirs("include", {public = true})