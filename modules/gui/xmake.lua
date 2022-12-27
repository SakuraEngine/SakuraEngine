shared_module("SkrGui", "SKR_GUI", engine_version)
    set_group("01.modules")
    public_dependency("SkrRenderGraph", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    -- add render graph shaders
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders", 
        dxil_outdir = "/../resources/shaders"})
    add_files("shaders/*.hlsl")