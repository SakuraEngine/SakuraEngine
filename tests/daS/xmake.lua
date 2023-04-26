add_requires("daScript 2023.4.26-skr")

target("daScriptVSCode")
    set_kind("phony")
    add_rules("@daScript/Standard", { 
        outdir = path.absolute(os.projectdir().."/.vscode") 
    })

target("SkrDAScript")
    set_kind("phony")
    add_deps("daScriptVSCode")
    add_rules("@daScript/Standard", { outdir = "." })

-- simple interpret
target("daSTestInterpret")
    set_kind("binary")
    set_group("05.tests/daS")
    add_deps("SkrDAScript")
    public_dependency("SkrRT", engine_version)
    add_packages("gtest", "daScript")
    add_files("daSTestInterpret/**.cpp")

-- AOT
target("daSTestAOT")
    set_kind("binary")
    set_group("05.tests/daS")
    add_rules("@daScript/AOT", {
        outdir = "./scripts",
        rootdir = os.curdir()
    })
    public_dependency("SkrRT", engine_version)
    add_packages("gtest", "daScript")
    add_files("daSTestAOT/**.das")
    add_files("daSTestAOT/**.cpp")

-- AOT
target("daSTestHybrid")
    set_kind("binary")
    set_group("05.tests/daS")
    add_rules("@daScript/Hybrid", {
        outdir = "./scripts"
    })
    public_dependency("SkrRT", engine_version)
    add_packages("gtest", "daScript")
    add_files("daSTestHybrid/aot/**.das", {aot = true})
    add_files("daSTestHybrid/script/**.das", {aot = false})
    add_files("daSTestHybrid/**.cpp")