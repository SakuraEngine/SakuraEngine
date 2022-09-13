imgui_sources_dir = "$(projectdir)/thirdparty/imgui"
imgui_includes_dir = "$(projectdir)/thirdparty/imgui/include"

target("imgui")
    set_kind("static")
    add_files(imgui_sources_dir.."/unitybuild.cpp")
    add_includedirs(imgui_includes_dir, {public=true})
    after_build(function(target)
        imgui_fontdir = path.join(os.projectdir(), "SDKs/SourceSansPro-Regular.ttf")
        os.cp(imgui_fontdir, path.join(target:targetdir(), "../resources/font").."/")
    end)

target("SkrImGui")
    add_rules("skr.module", {api = "SKR_IMGUI"})
    add_deps("imgui", "SkrRT", "SkrRenderGraph")
    add_includedirs("include", {public=true})
    add_includedirs(imgui_includes_dir, {public=true})
    add_files("src/build.*.cpp")
    -- add render graph shaders
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders", 
        dxil_outdir = "/../resources/shaders"})
    add_files("shaders/*.hlsl")