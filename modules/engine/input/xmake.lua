shared_module("SkrInput", "SKR_INPUT")
    public_dependency("SkrRT")
    add_includedirs("include", {public=true})
    skr_unity_build()
    if (is_plat("windows")) then
        add_defines("SKR_INPUT_USE_GAME_INPUT", {public=false})
        add_files("src/game_input/**.cpp")

        -- input links
        -- add_linkdirs("lib/x64", {public=true})
        -- if (is_mode("release")) then
        --     add_linkdirs("lib/x64/Release", {public=true})
        -- else
        --     add_linkdirs("lib/x64/Debug", {public=true})
        -- end
        -- add_links("GameInput", "xgameruntime", "Microsoft.Xbox.Services.141.GDK.C.Thunks", {public=false})
    end
    add_files("src/*.cpp")
    -- common layer implemented with SDL3
    add_files("src/common/**.cpp")
    add_files("src/sdl3/**.cpp")
    add_deps("SDL3")
