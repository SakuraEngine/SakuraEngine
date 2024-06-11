codegen_component("GameRuntime", { api = "GAME_RUNTIME", rootdir = "include/GameRuntime" })
    add_files("include/**.h")
    add_files("include/**.hpp")

shared_module("GameRuntime", "GAME_RUNTIME", engine_version)
    set_group("04.examples/application")
    public_dependency("SkrRenderer", engine_version)
    public_dependency("SkrImGui", engine_version)
    public_dependency("SkrInputSystem", engine_version)
    public_dependency("SkrAnim", engine_version)
    -- public_dependency("SkrTweak", engine_version)
    -- public_dependency("SkrInspector", engine_version)
    add_includedirs("modules/game_runtime/include/", { public=true })
    add_includedirs("./../../common", {public = false})
    add_files("modules/game_runtime/src/**.cpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch})

executable_module("Game", "GAME", engine_version)
    set_group("04.examples/application")
    set_exceptions("no-cxx")
    public_dependency("GameRuntime", engine_version)
    add_rules("utils.install_resources", {
        extensions = {".lua"},
        outdir = "/../resources", 
        rootdir = os.curdir().."/script"})
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/Game",
        dxil_outdir = "/../resources/shaders/Game"})
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_includedirs("./../../common", {public = false})
    add_files("src/**.cpp")
    add_files("shaders/**.hlsl")
    add_files("script/**.lua")
