target("OpenGUI_DemoResources")
    set_group("04.examples/application")
    add_rules("skr.static_module", {api = "OPENGUI_DEMO_RESOURCES"})
    add_includedirs("common", {public = true})
    add_rules("utils.install-resources", {
        extensions = {".png"},
        outdir = "/../resources/OpenGUI", 
        rootdir = os.curdir().."/common"})
    add_includedirs("./../../common", {public = true})
    add_includedirs("common", {public = true})
    add_files("common/**.png")
    add_files("common/**.cpp")
    public_dependency("SkrInput", engine_version)
    public_dependency("SkrGui", engine_version)
    public_dependency("SkrGuiRenderer", engine_version)
    
includes("gdi/xmake.lua")
includes("robjects/xmake.lua")