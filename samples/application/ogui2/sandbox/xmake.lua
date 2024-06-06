codegen_component("OGUI_Sandbox", { api = "OGUI_SANDBOX", rootdir = "src" })
    add_files("src/**.hpp")
    
executable_module("OGUI_Sandbox", "OGUI_SANDBOX", engine_version)
    set_group("04.examples/application")
    set_kind("binary")
    add_deps("OpenGUI_DemoResources")
    public_dependency("SkrInputSystem", "0.1.0")
    public_dependency("SkrImGui", "0.1.0")
    public_dependency("SkrGui", "0.1.0")
    public_dependency("SkrGuiRenderer", "0.1.0")

    add_includedirs("./../common", {public = false})
    add_files("src/*.cpp") 