--[[
target("static0")
    set_kind("static")
    add_rules("skr.module", {api = "STATIC0", version = engine_version})
    public_dependency("SkrRT", engine_version)
    add_files("test/static0.cpp")

target("dynamic0")
    set_kind("shared")
    add_rules("skr.module", {api = "DYNAMIC0", version = engine_version})
    public_dependency("SkrRT", engine_version)
    add_files("test/dynamic0.cpp")

target("dynamic1")
    set_kind("shared")
    add_rules("skr.module", {api = "DYNAMIC1", version = engine_version})
    public_dependency("SkrRT", engine_version)
    add_files("test/dynamic1.cpp")

target("dynamic2")
    set_kind("shared")
    add_rules("skr.module", {api = "DYNAMIC2", version = engine_version})
    public_dependency("SkrRT", engine_version)
    add_files("test/dynamic2.cpp")

target("dynamic3")
    set_kind("shared")
    add_rules("skr.module", {api = "DYNAMIC3", version = engine_version})
    public_dependency("SkrRT", engine_version)
    add_files("test/dynamic3.cpp")

target("module-test")
    set_kind("binary")
    add_rules("skr.module", {api = "MODULE_TEST", version = engine_version})
    public_dependency("SkrRT", engine_version)
    public_dependency("static0", engine_version)
    public_dependency("dynamic0", engine_version)
    public_dependency("dynamic1", engine_version)
    public_dependency("dynamic2", engine_version)
    public_dependency("dynamic3", engine_version)
    -- actually there is no need to strongly link these dynamic modules
    add_packages("gtest")
    add_files("test/main.cpp")
]]--