tracy_includes_dir = "$(projectdir)/thirdparty/tracy"

target("SkrRoot")
    add_includedirs(tracy_includes_dir, {public = true})

target("SkrCompileFlags")
    add_defines("TRACY_IMPORTS", "TRACY_ON_DEMAND", "TRACY_FIBERS", {public = true})
    if (is_config("use_tracy", "enable")) then
        add_defines("TRACY_OVERRIDE_ENABLE", {public = true})
    end
    if (is_config("use_tracy", "disable")) then
        add_defines("TRACY_OVERRIDE_DISABLE", {public = true})
    end

target("tracyclient")
    -- version("0.9.2alpha")
    set_group("00.thirdparty")
    set_kind("shared")
    add_files("$(projectdir)/thirdparty/tracy/TracyClient.cpp")
    add_includedirs("$(projectdir)/thirdparty/tracy", {public = true})
    add_defines("TRACY_IMPORTS", "TRACY_ON_DEMAND", "TRACY_FIBERS", {public = true})
    add_defines("TRACY_EXPORTS", "TRACY_ENABLE", {public = false})