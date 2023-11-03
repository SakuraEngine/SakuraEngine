target("MathTest")
    set_group("05.tests/base")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("math/math.cpp")

target("GraphTest")
    set_group("05.tests/base")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrRenderGraph", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("graph/graph.cpp")

target("VFSTest")
    set_group("05.tests/base")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("vfs/main.cpp")

target("SerdeTest")
    set_group("05.tests/base")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_files("serde/main.cpp")

target("ECSTest")
    set_group("05.tests/base")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("ecs/main.cpp")

target("MDBTest")
    set_group("05.tests/base")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrLightningStorage", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("mdb/main.cpp")

-- TODO. remove rtti test module when finish RTTR system 
-- shared_module("RTTITestTypes", "RTTI_TEST_TYPES", engine_version)
--     set_group("05.tests/framework")
--     add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
--     public_dependency("SkrRT", engine_version)
--     add_deps("SkrTestFramework", {public = false})
--     add_files("types/types.cpp")
--     add_rules("c++.codegen", {
--         files = {"types/**.h", "types/**.hpp"},
--         rootdir = "types/",
--         api = "RTTI_TEST_TYPES"
--     })

-- target("RTTITest")
--     set_group("05.tests/base")
--     set_kind("binary")
--     public_dependency("SkrRT", engine_version)
--     public_dependency("RTTITestTypes", engine_version)
--     add_deps("SkrTestFramework", {public = false})
--     add_rules("c++.codegen", {
--         files = {"rtti/**.h", "rtti/**.hpp"},
--         rootdir = "rtti/",
--         api = "RTTI_TEST"
--     })
--     add_files("rtti/rtti.cpp")

-- target("SPtrTest")
--     set_group("05.tests/base")
--     set_kind("binary")
--     public_dependency("SkrRT", engine_version)
--     public_dependency("RTTITestTypes", engine_version)
--     add_deps("SkrTestFramework", {public = false})
--     add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
--     add_files("sptr/**.cpp")

executable_module("RTTRTest", "RTTR_TEST", engine_version)
    set_group("05.tests/base")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_rules("c++.codegen", {
        files = {"rttr/**.h", "rttr/**.hpp"},
        rootdir = "rttr/",
        api = "RTTR_TEST"
    })
    add_files("rttr/rttr_test.cpp")

    
executable_module("TraitTest", "TRAIT_TEST", engine_version)
    set_group("05.tests/base")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_rules("c++.codegen", {
        files = {"trait/**.h", "trait/**.hpp"},
        rootdir = "trait/",
        api = "TRAIT_TEST"
    })
    add_files("trait/trait_test.cpp")


-- includes("module/xmake.lua")
-- includes("wasm/xmake.lua")