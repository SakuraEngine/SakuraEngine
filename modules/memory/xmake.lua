target("SkrMemory")
    set_group("01.modules")
    set_kind("shared")
    -- add source files
    add_files("src/build.*.c")
    add_files("src/build.*.cpp")
    add_deps("SkrBase", "SkrProfile", {public = true, inherit = true})
    add_deps("mimalloc", {public = false})
    add_includedirs("include", {public = true})
    add_defines("SKR_MEMORY_IMPL", {public = false})
    -- add OpenString defines
    add_defines("OPEN_STRING_API=SKR_MEMORY_API", {public = true})