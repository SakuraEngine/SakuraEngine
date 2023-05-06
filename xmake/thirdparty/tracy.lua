tracy_includes_dir = "$(projectdir)/thirdparty/tracy"
table.insert(include_dir_list, tracy_includes_dir)
table.insert(defs_list, "TRACY_IMPORTS")
table.insert(defs_list, "TRACY_ON_DEMAND")
table.insert(defs_list, "TRACY_FIBERS")

if (is_config("use_tracy", "enable")) then
    table.insert(defs_list, "TRACY_OVERRIDE_ENABLE")
end

if (is_config("use_tracy", "disable")) then
    table.insert(defs_list, "TRACY_OVERRIDE_DISABLE")
end

target("tracyclient")
    -- version("0.9.2alpha")
    set_group("00.thirdparty")
    set_kind("shared")
    add_files("$(projectdir)/thirdparty/tracy/TracyClient.cpp")
    add_includedirs("$(projectdir)/thirdparty/tracy", {public = true})
    add_defines("TRACY_IMPORTS", "TRACY_ON_DEMAND", "TRACY_FIBERS", {public = true})
    add_defines("TRACY_EXPORTS", "TRACY_ENABLE", {public = false})