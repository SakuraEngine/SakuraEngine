-- basement, can not be culled
includes("mimalloc")
includes("SDL3")
includes("rtm")

skr_includes_with_cull("gamenetworkingsockets", function()
    includes("gamenetworkingsockets")
end)
skr_includes_with_cull("vulkan", function()
    includes("vulkan")
end)
skr_includes_with_cull("libhv", function()
    includes("libhv")
end)

skr_includes_with_cull("v8", function()
    includes('v8')
end)

skr_includes_with_cull("cef", function()
    includes('cef')
end)

skr_includes_with_cull("openssl", function()
    includes("openssl")
end)