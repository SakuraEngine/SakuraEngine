target("CGPUTests")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-rspool-test",
        dxil_outdir = "/../resources/shaders/cgpu-rspool-test"})
    set_kind("binary")
    set_group("05.vid_tests/cgpu")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_packages("catch2", {public = true})
    add_files("Common.cpp")
    -- Device Initialize
    add_files("DeviceInitialize/**.cpp")
    -- Device Extensions
    add_files("Exts/**.cpp")
    -- QueueOps
    add_files("QueueOperations/QueueOperations.cpp")
    -- RSPool
    add_files("RootSignaturePool/RootSignaturePool.cpp")
    add_files("RootSignaturePool/shaders/**.hlsl")
    -- ResourceCreation
    add_files("ResourceCreation/ResourceCreation.cpp")
    -- SwapChainCreation
    add_files("SwapChainCreation/SwapChainCreation.cpp")