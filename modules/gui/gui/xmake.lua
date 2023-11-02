add_requires("freetype >=2.13.0-skr", {system = false})
add_requires("icu >=72.1.0-skr", {system = false})
add_requires("harfbuzz >=7.1.0-skr", {system = false})

add_requires("nanovg >=0.1.0-skr", {system = false})

shared_module("SkrGui", "SKR_GUI", engine_version)
    set_group("01.modules")
    add_packages("freetype", "icu", "harfbuzz")
    add_packages("nanovg")
    public_dependency("SkrRT", engine_version)

    -- reflection
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/SkrGui",
        api = "SKR_GUI"
    })

    -- unity build & pch
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    set_pcxxheader("src/pch.hpp")
    add_includedirs("include", {public = true})
    add_includedirs("src", {public = false})
    add_files("src/*.cpp")
    add_files("src/math/**.cpp")
    add_files("src/framework/**.cpp", {unity_group = "framework"})
    add_files("src/render_objects/**.cpp", {unity_group = "render_objects"})
    add_files("src/widgets/**.cpp", {unity_group = "widgets"})
    add_files("src/dev/**.cpp", {unity_group = "dev"})
    
    add_files("src/backend/*.cpp")
    add_files("src/backend/paragraph/*.cpp", {unity_group  = "text"})
    add_files("src/backend/text_server/*.cpp", {unity_group  = "text"})
    add_files("src/backend/text_server_adv/*.cpp", {unity_group  = "text_adv"})
    
    remove_files("src/dev/deprecated/**.cpp")
    if (is_plat("windows")) then
        add_cxflags("/wd4267", "/wd4244", "/wd4018","/source-charset:utf-8", {public=false})
    end