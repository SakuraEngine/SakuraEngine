shared_module("SkrInput", "SKR_INPUT", engine_version)
    set_group("01.modules")
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    if (is_plat("windows")) then
        add_files("src/game_input/**.cpp")
        add_linkdirs("lib/x64", {public=true})
        add_links("GameInput", "xgameruntime", {public=true})
        add_rules("utils.install-libs", { libnames = {"GameInput"} })
    else
        add_files("src/fallback/**.cpp")
    end
