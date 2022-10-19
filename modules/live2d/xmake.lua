target("SkrLive2D")
    set_group("01.modules")
    add_rules("skr.module", {api = "SKR_LIVE2D", version = engine_version})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/", disable_meta = true,
        api = "SKR_LIVE2D"
    })
    public_dependency("SkrImageCoder", engine_version)
    public_dependency("SkrRenderer", engine_version)
    add_includedirs("include", "CubismNativeCore/include", {public=true})
    add_includedirs("src/Framework", {public=false})
    add_files("src/**.cpp")
    add_rules("c++.noexception")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    -- link to cubism core
    if (is_os("windows")) then 
        add_linkdirs("CubismNativeCore/lib/windows/x86_64/142", {public=true})
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
    -- add live2d shaders
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders", 
        dxil_outdir = "/../resources/shaders"})
    add_files("shaders/*.hlsl")