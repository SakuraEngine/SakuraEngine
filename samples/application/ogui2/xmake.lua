target("OpenGUI_DemoResources")
    set_kind("static")
    set_group("04.examples/application")
    add_includedirs("common", {public = true})
    add_deps("AppSampleCommon")
    add_includedirs("common", {public = true})
    add_files("common/**.cpp")
    public_dependency("SkrInput")
    public_dependency("SkrGui")
    public_dependency("SkrGuiRenderer")

    -- install
    skr_install_rule()
    skr_install("files", {
        name = "ogui-demo-resources",
        files = {"common/**.png"},
        out_dir = "../resources/OpenGUI",
        root_dir = "common"
    })

-- includes("gdi/xmake.lua")
-- includes("robjects/xmake.lua")
includes("sandbox/xmake.lua")