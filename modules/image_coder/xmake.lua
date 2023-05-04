add_requires("zlib >=1.2.8-skr", {system = false})

shared_module("SkrImageCoder", "SKR_IMAGE_CODER", engine_version)
    set_group("01.modules")
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_packages("zlib")
    -- jpeg
    add_includedirs("include", "turbojpeg", {public=true})
    if (is_os("windows")) then 
        add_linkdirs("lib/windows/x64", {public=true})

        add_includedirs("include", {public=true})
        add_includedirs("libpng/1.5.2", {public=true})
        if (is_mode("asan")) then
            add_links("libpng15_static", {public=true})
        elseif (is_mode("release")) then
            add_links("libpng15_static", {public=true})
        else
            add_links("libpng15_staticd", {public=true})
        end
        add_links("turbojpeg_static", {public=true})
    end
    -- png
    if (is_os("macosx")) then 
        add_linkdirs("lib/macos/x86_64", {public=true})

        add_includedirs("include", "libpng/1.5.27", {public=true})
        add_links("png", {public=true})
        add_links("turbojpeg", {public=true})
    end