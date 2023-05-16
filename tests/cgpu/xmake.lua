target("CGPUSwapchainTest")
    set_kind("binary")
    set_group("05.vid_tests/cgpu")
    public_dependency("SkrRT", engine_version)
    add_packages("gtest")
    add_files("SwapChainCreation/SwapChainCreation.cpp")

target("CGPUResourceTest")
    set_kind("binary")
    set_group("05.vid_tests/cgpu")
    public_dependency("SkrRT", engine_version)
    add_packages("gtest")
    add_files("ResourceCreation/ResourceCreation.cpp")

target("CGPURSPoolTest")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-rspool-test",
        dxil_outdir = "/../resources/shaders/cgpu-rspool-test"})
    set_kind("binary")
    set_group("05.vid_tests/cgpu")
    public_dependency("SkrRT", engine_version)
    add_packages("gtest")
    add_files("RootSignaturePool/RootSignaturePool.cpp")
    add_files("RootSignaturePool/shaders/**.hlsl")