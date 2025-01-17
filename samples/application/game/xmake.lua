includes("modules/game_runtime/xmake.lua")

executable_module("Game", "GAME")
    set_group("04.examples/application")
    set_exceptions("no-cxx")
    public_dependency("GameRuntime")
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_includedirs("./../../common", {public = false})
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

