target("cgpu-swapchain-test")
    set_kind("binary")
    add_deps("SkrRT")
    add_packages("gtest")
    add_files("SwapChainCreation/SwapChainCreation.cpp")
    set_languages("c++17")

target("cgpu-resource-test")
    set_kind("binary")
    add_deps("SkrRT")
    add_packages("gtest")
    add_files("ResourceCreation/ResourceCreation.cpp")
    set_languages("c++17")

target("cgpu-rspool-test")
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/cgpu-rspool-test",
        dxil_outdir = "/../resources/shaders/cgpu-rspool-test"})
    set_kind("binary")
    add_deps("SkrRT")
    add_packages("gtest")
    add_files("RootSignaturePool/RootSignaturePool.cpp")
    add_files("RootSignaturePool/shaders/**.hlsl")
    set_languages("c++17")