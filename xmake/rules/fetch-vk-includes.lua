rule("utils.fetch-vk-includes")
    before_build(function (target)
        -- find vulkan includes dir
        import("lib.detect.find_path")
        local vk_from_env = os.getenv("VULKAN_SDK")
        if (vk_from_env ~= nil) then
            if (os.host() == "windows") then 
                vk_include_dirs = path.join(vk_from_env, "/Include")
            else
                vk_include_dirs = path.join(vk_from_env, "/include")
            end
        else
            vk_include_dirs = find_path("vulkan/vulkan.h", { "/usr/include", "/usr/local/include", "$(env PATH)"})
        end
        print("found vulkan include dir: "..vk_include_dirs)
        target:add("includedirs", vk_include_dirs)
    end)