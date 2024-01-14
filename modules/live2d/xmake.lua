static_component("CubismFramework", "SkrLive2D", { public = false })
    set_optimize("fastest")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    public_dependency("SkrBase", engine_version)
    add_includedirs("CubismNativeCore/include", "CubismFramework", "CubismFramework/Framework", {public=false})
    set_pcxxheader("CubismFramework/pch.hpp")
    add_files("CubismFramework/Renderer/**.cpp", "CubismFramework/Framework/**.cpp")
    -- link to cubism core
    if (is_os("windows")) then 
        add_linkdirs("CubismNativeCore/lib/windows/x86_64/142", {public=true})
        if (is_mode("asan")) then
            set_runtimes("MD") -- csmiPlatformDependentLogPrint uses freopen
        end
        if (is_mode("release")) then
            add_links("Live2DCubismCore_MD", {public=true})
        else
            add_links("Live2DCubismCore_MDd", {public=true})
        end
    end
    if (is_os("macosx")) then 
        add_linkdirs("CubismNativeCore/lib/macos/x86_64", {public=true})
        add_links("Live2DCubismCore", {public=true})
    end

shared_module("SkrLive2D", "SKR_LIVE2D", engine_version)
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    public_dependency("SkrImageCoder", engine_version)
    public_dependency("SkrRenderer", engine_version)
    add_includedirs("include", {public=true})
    add_includedirs("CubismNativeCore/include", "CubismFramework", "CubismFramework/Framework", {public=false})
    set_pcxxheader("src/pch.hpp")
    add_files("src/*.cpp")
    -- add live2d shaders
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders", 
        dxil_outdir = "/../resources/shaders"})
    add_files("shaders/*.hlsl")