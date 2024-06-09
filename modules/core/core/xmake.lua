add_requires("lemon v1.3.1")
add_requires("parallel-hashmap >=1.3.11-skr")
add_requires("yyjson v0.9.0")

static_component("SkrGuid", "SkrCore")
    set_optimize("fastest")
    add_deps("SkrBase")
    add_files("src/guid/build.*.cpp")

static_component("SkrDependencyGraph", "SkrCore")
    set_optimize("fastest")
    add_deps("SkrBase")
    add_packages("lemon", {public = false, inherit = false})
    add_files("src/graph/build.*.cpp")

static_component("SkrString", "SkrCore")
    set_optimize("fastest")
    add_deps("SkrBase")
    add_defines("OPEN_STRING_API=", {public = true})
    add_files("src/string/build.*.cpp")

static_component("SkrSimpleAsync", "SkrCore")
    set_optimize("fastest")
    add_deps("SkrBase")
    add_files("src/async/build.*.cpp")

static_component("SkrJson", "SkrCore")
    add_deps("SkrBase")
    add_files("src/json/build.*.cpp")
    add_packages("yyjson", {public = false})

static_component("SkrSerde", "SkrCore")
    add_deps("SkrJson")
    add_packages("parallel-hashmap", {public = true, inherit = true})
    add_files("src/serde/build.*.cpp")

shared_module("SkrCore", "SKR_CORE", engine_version)
    -- add source files
    add_packages("parallel-hashmap", {public = true, inherit = true})
    add_deps("SkrProfile", {public = true, inherit = true})
    add_deps("mimalloc", {public = false})
    add_includedirs("include", {public = true})
    add_defines("SKR_MEMORY_IMPL", {public = false})
    -- core
    add_files("src/core/build.*.c", "src/core/build.*.cpp")
    -- rttr
    add_files("src/rttr/build.*.cpp")
    -- serde
    add_files("src/serde/export.*.cpp")