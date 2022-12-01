shared_module("GameRuntime", "GAME_RUNTIME", engine_version)
    set_group("04.examples/application")
    add_rules("c++.codegen", {
        files = {"modules/game_runtime/include/**.h", "modules/game_runtime/include/**.hpp"},
        rootdir = "modules/game_runtime/include/GameRuntime",
        api = "GAME_RUNTIME"
    })
    public_dependency("SkrRenderer", engine_version)
    public_dependency("SkrImGui", engine_version)
    public_dependency("SkrInputSystem", engine_version)
    public_dependency("SkrAnim", engine_version)
    public_dependency("SkrTweak", engine_version)
    public_dependency("SkrInspector", engine_version)
    add_includedirs("modules/game_runtime/include/", { public=true })
    add_files("modules/game_runtime/src/**.cpp")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})

executable_module("Game", "GAME", engine_version)
    set_group("04.examples/application")
    set_exceptions("no-cxx")
    public_dependency("GameRuntime", engine_version)
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/Game",
        dxil_outdir = "/../resources/shaders/Game"})
    add_files("src/main.cpp", "src/render_resources.cpp", "src/render_effects.cpp",  "src/game_module.cpp")
    add_files("shaders/**.hlsl")
    if (is_os("windows")) then 
        add_files("/../../../resources/windows/sakura.rc")
    end