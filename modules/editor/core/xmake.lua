codegen_component("SkrEditorCore", { api = "SKR_EDITOR_CORE", rootdir = "include/SkrEditorCore" })
    add_files("include/**.hpp")

shared_module("SkrEditorCore", "SKR_EDITOR_CORE")
    public_dependency("SkrRT")
    
    add_includedirs("include", {public = true})
    add_files("src/*.cpp")