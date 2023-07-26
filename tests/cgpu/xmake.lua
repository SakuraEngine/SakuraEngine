target("CGPUTests")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-rspool-test",
        dxil_outdir = "/../resources/shaders/cgpu-rspool-test"})
    set_kind("binary")
    set_group("05.vid_tests/cgpu")
    public_dependency("SkrRT", engine_version)
    add_deps("SkrTestFramework", {public = false})
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_files(
        "Common.cpp",
        -- Device Initialize
        "DeviceInitialize/**.cpp",
        -- Device Extensions
        "Exts/**.cpp",
        -- QueueOps
        "QueueOperations/QueueOperations.cpp",
        -- RSPool
        "RootSignaturePool/RootSignaturePool.cpp",
        "RootSignaturePool/shaders/**.hlsl",
        -- ResourceCreation
        "ResourceCreation/ResourceCreation.cpp",
        -- SwapChainCreation
        "SwapChainCreation/SwapChainCreation.cpp"
    , {unity_group = "CPUTests"})