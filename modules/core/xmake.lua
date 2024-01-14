add_requires("lemon 1.3.1")
add_requires("parallel-hashmap >=1.3.11-skr")
add_requires("simdjson >=3.0.0-skr")

static_component("SkrGuid", "SkrCore")
    set_optimize("fastest")
    public_dependency("SkrBase", engine_version)
    add_files("src/guid/build.*.cpp")

static_component("SkrDependencyGraph", "SkrCore")
    set_optimize("fastest")
    public_dependency("SkrBase", engine_version)
    add_packages("lemon", {public = false, inherit = false})
    add_files("src/graph/build.*.cpp")

static_component("SkrString", "SkrCore")
    set_optimize("fastest")
    add_defines("OPEN_STRING_API=", {public = true})
    add_files("src/string/build.*.cpp")

static_component("SkrSimpleAsync", "SkrCore")
    set_optimize("fastest")
    add_files("src/async/build.*.cpp")

shared_module("SkrCore", "SKR_CORE", engine_version)
    -- add source files
    add_packages("simdjson", {public = true, inherit = true})
    add_packages("parallel-hashmap", {public = true, inherit = true})
    add_deps("SkrProfile", {public = true, inherit = true})
    add_deps("mimalloc", {public = false})
    add_includedirs("include", {public = true})
    add_defines("SKR_MEMORY_IMPL", {public = false})
    set_pcxxheader("src/core/pch.hpp")
    add_files("src/core/build.*.c", "src/core/build.*.cpp")