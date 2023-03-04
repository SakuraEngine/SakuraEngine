shared_module("SkrGuiRenderer", "SKR_GUI_RENDERER", engine_version)
    set_group("01.modules")
    public_dependency("SkrGui", engine_version)
    public_dependency("SkrRenderGraph", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    -- add render graph shaders
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/GUI", 
        dxil_outdir = "/../resources/shaders/GUI"})
    add_files("shaders/*.hlsl")
    if (is_os("windows") or is_os("macosx")) then 
        public_dependency("SkrImageCoder", engine_version)
        add_defines("SKR_GUI_RENDERER_USE_IMAGE_CODER")
    end