target("SDL3")
    set_kind("headeronly")

    -- add packages
    add_includedirs("include", {public = true})
    
    -- download prebuilt binaries
    skr_install_rule()
    skr_install("download", {
        name = "SDL_3.2.12",
        install_func = "sdk",
    })

    -- add links
    if is_plat("windows") then
        add_links("SDL3", {public = true})
    else
        -- add_links("SDL3-3.2.12", {public = true})
    end

    -- for clangd
    add_files("./_dummy_SDL3_for_clangd.cpp")

    -- avoid define main
    add_defines("SDL_MAIN_HANDLED=1", {public = true})