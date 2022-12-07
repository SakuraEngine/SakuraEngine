target("imgui")
    set_group("00.thirdparty")
    if(has_config("shipping_one_archive")) then
        add_defines("RUNTIME_ALL_STATIC")
    else
        add_defines("RUNTIME_SHARED")
    end
    add_includedirs("imgui/include", {public=true})
    after_build(function(target)
        imgui_fontdir = path.join(os.projectdir(), "SDKs/SourceSansPro-Regular.ttf")
        os.cp(imgui_fontdir, path.join(target:targetdir(), "../resources/font").."/")
    end)
    -- contains in imgui-$(plat)-$(arch).zip
    if (is_os("windows")) then 
         set_kind("headeronly")    
         if (is_mode("release")) then
             add_links(sdk_libs_dir.."imgui", {public=true} )
         else
             add_links(sdk_libs_dir.."imguid", {public=true} )
         end
    else
        set_kind("static")
        -- compiles very slow, so we use precompiled version for this optimization level
        -- set_optimize("fastest")
        add_files("imgui/build.*.cpp")
    end

target("cimgui")
    set_group("00.thirdparty")
    set_kind("static")
    set_optimize("fastest")
    add_deps("imgui", {public=true})
    add_includedirs("imgui", {public=false})
    add_includedirs("dear_bindings/cimgui/include", {public=true})
    -- contains in imgui-$(plat)-$(arch).zip
    if (is_os("windows")) then 
         set_kind("headeronly")    
         if (is_mode("release")) then
             add_links(sdk_libs_dir.."cimgui", {public=true} )
         else
             add_links(sdk_libs_dir.."cimguid", {public=true} )
         end
    else
        set_kind("static")
        -- compiles very slow, so we use precompiled version for this optimization level
        -- set_optimize("fastest")
        add_files("dear_bindings/cimgui/src/cimgui.cpp")
    end

shared_module("SkrImGui", "SKR_IMGUI", engine_version)
    set_group("01.modules")
    public_dependency("SkrRenderGraph", engine_version)
    add_deps("cimgui")
    add_includedirs("include", {public=true})
    add_files("src/build.*.cpp")
    -- add render graph shaders
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders", 
        dxil_outdir = "/../resources/shaders"})
    add_files("shaders/*.hlsl")