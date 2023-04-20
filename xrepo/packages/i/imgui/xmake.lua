package("imgui")
    set_homepage("https://github.com/ocornut/imgui")
    set_description("Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies")
    set_license("MIT")
--[[
    set_homepage("https://github.com/cimgui/cimgui")
    set_description("c-api for imgui")
    set_license("MIT")
]]--
    
    add_versions("1.89.0-skr", "395f9d71cc5fb76cd6d8ab2f354de6d37466386e84e2abce2feabee2c4e562e4")
    add_configs("runtime_shared", {description = "Import gctx from dll.", default = false, type = "boolean"})

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "cimgui"), ".")
        os.cp(path.join(package:scriptdir(), "port", "imgui"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        configs.runtime_shared = true
        if package:config("runtime_shared") then
            configs.runtime_shared = true
        else
            configs.runtime_shared = false
            package:add("defines", "RUNTIME_ALL_STATIC", { public=true })
        end
        
        import("package.tools.xmake").install(package, configs)
    end)
    
    on_test(function (package)
        assert(package:has_cxxtypes("ImGuiIO", {includes = "imgui/imgui.h", configs = {languages = "cxx17"}}))
    end)
