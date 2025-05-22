codegen_component("SkrGuiRenderer", { api = "SKR_GUI_RENDERER", rootdir = "include/SkrGuiRenderer" })
    add_files("include/**.hpp")

shared_module("SkrGuiRenderer", "SKR_GUI_RENDERER")
    public_dependency("SkrGui")
    public_dependency("SkrRenderGraph")
    add_includedirs("include", {public=true})
    add_includedirs("src", {public=false})
    add_files("src/**.cpp")
    -- add render graph shaders
    skr_unity_build()
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/GUI", 
        dxil_outdir = "/../resources/shaders/GUI"}
    )
    add_files("shaders/*.hlsl")
    if (is_os("windows") or is_os("macosx")) then 
        public_dependency("SkrImageCoder")
        add_defines("SKR_GUI_RENDERER_USE_IMAGE_CODER")
    end
    add_deps("AppSampleCommon")