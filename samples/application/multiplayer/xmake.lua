set_project("Multiplayer")
add_requires("lz4")

shared_module("MPShared", "MP_SHARED", engine_version)
    set_group("04.examples/network")
    add_rules("c++.codegen", {
        files = {"modules/mpshared/include/MPShared/**.h", "modules/mpshared/include/MPShared/**.hpp"},
        rootdir = "modules/mpshared/include/MPShared",
        api = "MP_SHARED"
    })
    add_includedirs("modules/mpshared/include/", { public=true })
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrScene", engine_version)
    add_files("modules/mpshared/src/**.cpp")
    if (is_os("windows")) then 
        add_syslinks("Ws2_32")
    end
    add_deps("gamenetworkingsockets")
    add_packages("lz4")
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})


executable_module("MPGame", "MP_GAME", engine_version)
    set_group("04.examples/network")
    add_rules("c++.codegen", {
        files = {"modules/mpgame/include/MPGame/**.h", "modules/mpgame/include/MPGame/**.hpp"},
        rootdir = "modules/mpgame/include/MPGame",
        api = "MP_GAME"
    })
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/Game",
        dxil_outdir = "/../resources/shaders/Game"})
    add_includedirs("modules/mpgame/include", {public=true})
    public_dependency("SkrRenderer", engine_version)
    public_dependency("SkrImGui", engine_version)
    public_dependency("SkrInputSystem", engine_version)
    public_dependency("SkrRT", engine_version)
    public_dependency("SkrAnim", engine_version)
    public_dependency("MPShared", engine_version)
    if (is_os("windows")) then 
        add_syslinks("Ws2_32")
    end
    add_deps("gamenetworkingsockets")
    add_files("modules/mpgame/shaders/**.hlsl")
    add_files("modules/mpgame/src/**.cpp")

executable_module("MPServer", "MP_SERVER", engine_version)
    set_group("04.examples/network")
    add_rules("c++.codegen", {
        files = {"modules/mpserver/include/MPServer/**.h", "modules/mpserver/include/MPServer/**.hpp"},
        rootdir = "modules/mpgame/include/MPServer",
        api = "MP_SERVER"
    })
    add_includedirs("modules/mpserver/include", {public=true})
    public_dependency("MPShared", engine_version)
    if (is_os("windows")) then 
        add_syslinks("Ws2_32")
    end
    add_deps("gamenetworkingsockets")
    add_files("modules/mpserver/src/**.cpp")
