target("OpenGUI_GDIDemo")
    set_group("04.examples/application")
    set_kind("binary")
    add_files("src/*.cpp") 
    public_dependency("SkrGui", "0.1.0")
    public_dependency("SkrGuiRenderer", "0.1.0")
    -- shaders
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/OpenGUI_GDIDemo",
        dxil_outdir = "/../resources/shaders/OpenGUI_GDIDemo"})
    add_files("shaders/**.hlsl")