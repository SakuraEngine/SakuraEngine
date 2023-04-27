shared_module("SkrInput", "SKR_INPUT", engine_version)
    set_group("01.modules")
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    if (is_plat("windows")) then
        add_defines("SKR_INPUT_USE_GAME_INPUT", {public=false})
        add_files("src/game_input/**.cpp")
        add_linkdirs("lib/x64", {public=true})
        if is_mode("release") or is_mode("one_archive") then
            add_linkdirs("lib/x64/Release", {public=true})
        else
            add_linkdirs("lib/x64/Debug", {public=true})
        end
        -- add_links("GameInput", "xgameruntime", "Microsoft.Xbox.Services.141.GDK.C.Thunks", {public=false})
    end
    add_files("src/*.cpp")
    -- common layer implemented with SDL2
    add_files("src/common/**.cpp")
    add_files("src/sdl2/**.cpp")
