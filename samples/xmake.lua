includes("application/xmake.lua")

includes("cgpu/xmake.lua")
includes("hotfix/xmake.lua")
includes("render_graph/xmake.lua")
includes("dcc/xmake.lua")

includes("websocket")
includes("imgui")
includes("common")

skr_includes_with_cull("cef", function()
    includes('cef')
end)

skr_includes_with_cull("v8", function()
    includes('v8_playground')
end)

skr_includes_with_cull("openssl", function()
    includes('openssl')
end)
