tracy_includes_dir = "$(projectdir)/thirdparty/tracy"
table.insert(include_dir_list, tracy_includes_dir)
table.insert(links_list, "$(buildir)/$(os)/$(arch)/$(mode)/TracyClient")

if (is_config("use_tracy", "enable")) then
    table.insert(defs_list, "TRACY_OVERRIDE_ENABLE")
end
if (is_config("use_tracy", "disable")) then
    table.insert(defs_list, "TRACY_OVERRIDE_DISABLE")
end