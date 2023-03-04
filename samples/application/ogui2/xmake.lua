target("OpenGUI_DemoResources")
    set_kind("object")
    add_rules("utils.install-resources", {
        extensions = {".png"},
        outdir = "/../resources/OpenGUI", 
        rootdir = os.curdir().."/resources"})
    add_files("resources/**.png")

includes("gdi/xmake.lua")