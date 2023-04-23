if(has_config("shipping_one_archive")) then
    add_requires("imgui =1.89.0-skr", { configs = { runtime_shared = false } })
else
    add_requires("imgui =1.89.0-skr", { configs = { runtime_shared = true } })
end

shared_module("SkrImGui", "SKR_IMGUI", engine_version)
    set_group("01.modules")
    public_dependency("SkrInput", engine_version)
    public_dependency("SkrRenderGraph", engine_version)
    add_packages("imgui", {public=true})
    add_includedirs("include", {public=true})
    add_defines("IMGUI_IMPORT= extern SKR_IMGUI_API", {public=false})
    add_files("src/build.*.cpp")
    -- add render graph shaders
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders", 
        dxil_outdir = "/../resources/shaders"})
    add_files("shaders/*.hlsl")
    after_build(function(target)
        local imgui_fontdir = path.join(os.projectdir(), "SDKs/SourceSansPro-Regular.ttf")
        os.cp(imgui_fontdir, path.join(target:targetdir(), "../resources/font").."/")
    end)