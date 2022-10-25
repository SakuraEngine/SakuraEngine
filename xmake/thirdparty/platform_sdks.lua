vk_includes_dir = "$(projectdir)/SDKs/vulkan"

target("vulkan")
    set_group("00.thirdparty")
    set_kind("headeronly")
    add_includedirs(vk_includes_dir, {public=true})