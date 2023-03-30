target("OpenGUI_DemoResources")
    set_group("04.examples/application")
    set_kind("static")
    add_includedirs("common", {public = true})
    add_rules("utils.install-resources", {
        extensions = {".png"},
        outdir = "/../resources/OpenGUI", 
        rootdir = os.curdir().."/common"})
    add_files("common/**.png")
    add_files("common/**.cpp")
    public_dependency("SkrInput", "0.1.0")
    public_dependency("SkrGui", "0.1.0")
    public_dependency("SkrGuiRenderer", "0.1.0")
    
includes("gdi/xmake.lua")
includes("robjects/xmake.lua")