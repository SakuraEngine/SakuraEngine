includes("core")

skr_includes_with_cull("editor", function ()
    includes("editor")
end)

skr_includes_with_cull("engine", function ()
    includes("engine")
end)

skr_includes_with_cull("render", function ()
    includes("render")
end)

skr_includes_with_cull("devtime", function ()
    includes("devtime")
end)

skr_includes_with_cull("gui", function ()
    includes("gui")
end)

skr_includes_with_cull("dcc", function ()
    includes("dcc")
end)

skr_includes_with_cull("tools", function ()
    includes("tools")
end)

skr_includes_with_cull("experimental", function ()
    -- includes("experimental/netcode")
    -- includes("experimental/physics")
    -- includes("experimental/tweak")
    -- includes("experimental/inspector")
    -- includes("experimental/runtime_exporter")
    -- includes("experimental/daScript")
end)