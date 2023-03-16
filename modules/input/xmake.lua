shared_module("SkrInput", "SKR_INPUT", engine_version)
    set_group("01.modules")
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    if (is_plat("windows")) then
        add_defines("SKR_INPUT_USE_GAME_INPUT", {public=false})
        add_files("src/game_input/**.cpp")
        add_linkdirs("lib/x64", {public=true})
        add_links("GameInput", {public=false})
        if (is_mode("release")) then
            add_linkdirs("lib/x64/Release", {public=true})
        else
            add_linkdirs("lib/x64/Debug", {public=true})
        end
        add_links("Microsoft.Xbox.Services.141.GDK.C.Thunks", {public=true})
    end
    add_files("src/*.cpp")
    add_files("src/common/**.cpp")
