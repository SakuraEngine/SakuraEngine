-- simple interpret
target("daSTestInterpret")
    set_kind("binary")
    set_group("05.tests/daS")
    public_dependency("SkrDAScript", engine_version)
    add_packages("gtest")
    add_files("daSTestInterpret/**.cpp")

-- AOT
target("daSTestAOT")
    set_kind("binary")
    set_group("05.tests/daS")
    add_rules("daScript", {
        outdir = "./scripts",
        rootdir = os.curdir()
    })
    public_dependency("SkrDAScript", engine_version)
    add_packages("gtest")
    add_files("daSTestAOT/**.das", {aot = true})
    add_files("daSTestAOT/**.cpp")
    
-- Annotation
target("daSTestAnnotationCompiler")
    set_kind("binary")
    set_group("05.tests/daS")
    public_dependency("SkrDAScript", engine_version)
    add_files("daSTestAnnotation/annotation_compiler.cpp")

target("daSTestAnnotation")
    set_kind("binary")
    set_group("05.tests/daS")
    add_rules("daScript", {
        outdir = "./scripts",
        rootdir = os.curdir()
    })
    public_dependency("SkrDAScript", engine_version)
    add_packages("gtest")
    add_files("daSTestAnnotation/**.das")
    add_files("daSTestAnnotation/test_annotation.cpp")

-- Coroutine
target("dasCo")
    set_kind("binary")
    set_group("05.tests/daS")
    add_rules("daScript", {
        outdir = "./scripts",
        rootdir = os.curdir()
    })
    public_dependency("SkrDAScript", engine_version)
    add_packages("gtest")
    add_files("dasCo/**.das")
    add_files("dasCo/**.cpp")

target("dasCoAOT")
    set_kind("binary")
    set_group("05.tests/daS")
    add_rules("daScript", {
        outdir = "./scripts",
        rootdir = os.curdir()
    })
    public_dependency("SkrDAScript", engine_version)
    add_packages("gtest")
    add_defines("AOT")
    add_files("dasCo/**.das", {aot = true})
    add_files("dasCo/**.cpp")

-- Stackwalk
target("dasStackwalk")
    set_kind("binary")
    set_group("05.tests/daS")
    add_rules("daScript", {
        outdir = "./scripts",
        rootdir = os.curdir()
    })
    public_dependency("SkrDAScript", engine_version)
    add_packages("gtest")
    add_files("dasStackwalk/**.das")
    add_files("dasStackwalk/**.cpp")

--[[
target("dasStackwalkAOT")
    set_kind("binary")
    set_group("05.tests/daS")
    add_rules("daScript", {
        outdir = "./scripts",
        rootdir = os.curdir()
    })
    public_dependency("SkrDAScript", engine_version)
    add_packages("gtest")
    add_defines("AOT")
    add_files("dasStackwalk/**.das", {aot = true})
    add_files("dasStackwalk/**.cpp")
]]--

-- Hybrid
target("daSTestHybrid")
    set_kind("binary")
    set_group("05.tests/daS")
    add_rules("daScript", {
        outdir = "./scripts",
        rootdir = os.curdir()
    })
    public_dependency("SkrDAScript", engine_version)
    add_packages("gtest")
    add_files("daSTestHybrid/aot/**.das", {aot = true})
    add_files("daSTestHybrid/script/**.das", {aot = false})
    add_files("daSTestHybrid/**.cpp")