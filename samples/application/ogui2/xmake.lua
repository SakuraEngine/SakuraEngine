static_library("OpenGUI_DemoResources", "OPENGUI_DEMO_RESOURCES", engine_version)
    set_group("04.examples/application")
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

-- TODO. GDI 满门抄斩
-- includes("gdi/xmake.lua")
-- includes("robjects/xmake.lua")
includes("sandbox/xmake.lua")