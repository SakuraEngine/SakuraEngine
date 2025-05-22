add_requires("lemon v1.3.1")
add_requires("parallel-hashmap >=1.3.11-skr")
add_requires("yyjson v0.9.0")

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

static_component("SkrArchive", "SkrCore")
    add_deps("SkrBase")
    add_files("src/archive/build.*.cpp")
    add_packages("yyjson", {public = false})

-- static_component("SkrSerde", "SkrCore")
--     add_deps("SkrArchive")
--     add_packages("parallel-hashmap", {public = true, inherit = true})
--     add_files("src/serde/build.*.cpp")

codegen_component("SkrCore", {api = "SKR_CORE", rootdir = "include"})
    add_files("include/**.hpp")

target("SkrCoreMeta")
    set_group("00.utilities")
    set_kind("headeronly")
    codegen_generator({
        scripts = {
            { file = "meta/basic.ts" },
            { file = "meta/rttr.ts" },
            { file = "meta/serialize.ts" },
            { file = "meta/proxy.ts" },
        }, 
        dep_files = {
            "meta/**.ts",
            "meta/**.py",
            "meta/**.mako"
        }
    })

shared_module("SkrCore", "SKR_CORE")
    -- add codegen generator
    add_deps("SkrCoreMeta")
    
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

    -- macosx files
    if (is_os("macosx")) then 
        add_files("src/**/build.*.mm")
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "IOKit", {public = true})
    end

    -- platform
    if (is_os("windows")) then 
        add_syslinks("advapi32", "user32", "shell32", "Ole32", "Shlwapi", {public = true})
    else
        add_syslinks("pthread")
    end

--[[
shared_pch("SkrCore")
    add_files("include/**.h")
    add_files("include/**.hpp")
]]--