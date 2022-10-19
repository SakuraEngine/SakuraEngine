imgui_sources_dir = "$(projectdir)/thirdparty/imgui"
imgui_includes_dir = "$(projectdir)/thirdparty/imgui/include"

target("imgui")
    set_group("00.thirdparty")
    set_kind("static")
    if(has_config("module_as_objects")) then
        add_defines("RUNTIME_ALL_STATIC")
    else
        add_defines("RUNTIME_SHARED")
    end
    add_files(imgui_sources_dir.."/unitybuild.cpp")
    add_includedirs(imgui_includes_dir, {public=true})
    after_build(function(target)
        imgui_fontdir = path.join(os.projectdir(), "SDKs/SourceSansPro-Regular.ttf")
        os.cp(imgui_fontdir, path.join(target:targetdir(), "../resources/font").."/")
    end)

target("SkrImGui")
    set_group("01.modules")
    add_rules("skr.module", {api = "SKR_IMGUI", version = engine_version})
    public_dependency("SkrRenderGraph", engine_version)
    add_deps("imgui")
    add_includedirs("include", {public=true})
    add_includedirs(imgui_includes_dir, {public=true})
    add_files("src/build.*.cpp")
    -- add render graph shaders
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders", 
        dxil_outdir = "/../resources/shaders"})
    add_files("shaders/*.hlsl")