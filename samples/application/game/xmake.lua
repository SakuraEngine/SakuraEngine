codegen_component("GameRuntime", { api = "GAME_RUNTIME", rootdir = "include/GameRuntime" })
    add_files("include/**.h")
    add_files("include/**.hpp")

shared_module("GameRuntime", "GAME_RUNTIME")
    set_group("04.examples/application")
    public_dependency("SkrRenderer")
    public_dependency("SkrImGui")
    public_dependency("SkrInputSystem")
    public_dependency("SkrAnim")
    -- public_dependency("SkrTweak")
    -- public_dependency("SkrInspector")
    add_includedirs("modules/game_runtime/include/", { public=true })
    add_deps("AppSampleCommon")
    add_files("modules/game_runtime/src/**.cpp")
    skr_unity_build()

executable_module("Game", "GAME")
    set_group("04.examples/application")
    set_exceptions("no-cxx")
    public_dependency("GameRuntime")
    skr_unity_build()
    add_deps("AppSampleCommon")
    add_files("src/**.cpp")

    -- install
    skr_install_rule()
    skr_install("files", {
        name = "game-script",
        files = {"script/**.lua"},
        out_dir = "../resources",
        root_dir = "script"
    })

    -- shaders
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/Game",
        dxil_outdir = "/../resources/shaders/Game"})
    add_files("shaders/**.hlsl")

