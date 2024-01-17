target("SkrProfile")
    set_group("01.libraries")
    set_kind("shared")
    add_includedirs("include", {public = true})
    add_files("internal/tracy/TracyClient.cpp") -- version("0.10.1alpha")
    if (is_config("use_profile", "enable")) then
        add_defines("SKR_PROFILE_OVERRIDE_ENABLE", {public = true})
    end
    if (is_config("use_profile", "disable")) then
        add_defines("SKR_PROFILE_OVERRIDE_DISABLE", {public = true})
    end