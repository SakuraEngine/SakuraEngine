target("MathTest")
    set_group("05.tests/runtime")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("math/math.cpp")

target("GoapTest")
    set_kind("binary")
    set_group("05.tests/runtime")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("goap/test_goap.cpp")

target("GraphTest")
    set_group("05.tests/runtime")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrRenderGraph", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("graph/graph.cpp")

target("VFSTest")
    set_group("05.tests/runtime")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("vfs/main.cpp")

target("SceneTest")
    set_group("05.tests/runtime")
    set_kind("binary")
    public_dependency("SkrScene", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_files("scene/main.cpp")

target("ECSTest")
    set_group("05.tests/runtime")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("ecs/main.cpp")

target("MDBTest")
    set_group("05.tests/runtime")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrLightningStorage", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_files("mdb/main.cpp")

executable_module("RTTRTest", "RTTR_TEST", engine_version)
    set_group("05.tests/runtime")
    set_kind("binary")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_rules("c++.codegen", {
        files = {"rttr/**.h", "rttr/**.hpp"},
        rootdir = "rttr/",
        api = "RTTR_TEST"
    })
    add_files("rttr/**.cpp")

executable_module("TraitTest", "TRAIT_TEST", engine_version, {exception = true})
    set_group("05.tests/runtime")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_rules("c++.codegen", {
        files = {"trait/**.h", "trait/**.hpp"},
        rootdir = "trait/",
        api = "TRAIT_TEST"
    })
    add_files("trait/trait_test.cpp")

if build_part("v8") then
    executable_module("V8Test", "V8_TEST", engine_version)
        set_group("05.tests/runtime")
        public_dependency("SkrV8", engine_version)
        -- add_deps("SkrTestFramework", {public = false})
        add_files("v8/hello_fucking_google.cpp")
end


-- includes("module/xmake.lua")
-- includes("wasm/xmake.lua")