target("SkrImageCoder")
    add_rules("skr.module", {api = "SKR_IMAGE_CODER"})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/", disable_reflection = true,
        api = "SKR_IMAGE_CODER"
    })
    add_deps("SkrRT", "zlib")
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    if (is_os("windows")) then 
        add_includedirs("include", "libpng/1.5.2", {public=true})
        add_linkdirs("lib/windows/x64", {public=true})
        if (is_mode("release")) then
            add_links("libpng15_static", {public=true})
        else
            add_links("libpng15_staticd", {public=true})
        end
    end
    if (is_os("macosx")) then 
        add_includedirs("include", "libpng/1.5.27", {public=true})
        add_linkdirs("lib/macos/x86_64", {public=true})
        add_links("png", {public=true})
    end