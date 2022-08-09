target("SkrLive2D")
    add_rules("skr.module", {api = "SKR_LIVE2D"})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/", disable_reflection = true,
        api = "SKR_LIVE2D"
    })
    add_deps("SkrRenderer")
    add_includedirs("include", "CubismNativeCore/include", {public=true})
    add_includedirs("src/Framework", {public=false})
    add_files("src/**.cpp")
    if (is_os("windows")) then 
        add_linkdirs("CubismNativeCore/lib/windows/x86_64/142")
        if (is_mode("release")) then
            add_links("Live2DCubismCore_MD")
        else
            add_links("Live2DCubismCore_MDd")
        end
    end
    if (is_os("macosx")) then 
        add_linkdirs("CubismNativeCore/lib/macos/x86_64")
        add_links("Live2DCubismCore")
    end