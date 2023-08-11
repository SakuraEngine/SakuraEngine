target("SkrCompileFlags")
    add_defines("TRACY_IMPORTS", "TRACY_ON_DEMAND", "TRACY_FIBERS", {public = true})
    if (is_config("use_profile", "enable")) then
        add_defines("SKR_PROFILE_OVERRIDE_ENABLE", {public = true})
    end
    if (is_config("use_profile", "disable")) then
        add_defines("SKR_PROFILE_OVERRIDE_DISABLE", {public = true})
    end

target("SkrProfile")
    set_group("00.thirdparty")
    set_kind("shared")
    -- version("0.9.2alpha")
    add_deps("SkrCompileFlags", {public = true})
    add_files("internal/tracy/TracyClient.cpp")
    add_includedirs("include", {public = true})
    add_defines("TRACY_EXPORTS", {public = false})