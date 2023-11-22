executable_module("OGUI_Sandbox", "OGUI_SANDBOX", engine_version)
    set_group("04.examples/application")
    set_kind("binary")
    add_includedirs("./../common", {public = false})
    add_files("src/*.cpp") 
    add_deps("OpenGUI_DemoResources")
   
    -- reflection
    add_rules("c++.codegen", {
        files = {"src/**.h", "src/**.hpp"},
        rootdir = "src",
        api = "OGUI_SANDBOX"
    })

    public_dependency("SkrInputSystem", "0.1.0")
    public_dependency("SkrImGui", "0.1.0")
    public_dependency("SkrGui", "0.1.0")
    public_dependency("SkrGuiRenderer", "0.1.0")