package("eastl")
    set_homepage("https://github.com/electronicarts/EASTL/")
    set_description("EASTL stands for Electronic Arts Standard Template Library.")
    set_license("BSD-3-Clause license")
    
    add_versions("3.20.2-skr", "7203ac3b94460535d610125050127282fdd9653408591ae7dbbee3d5cc364636")
    add_configs("runtime_shared", {description = "Import allocator from dll.", default = false, type = "boolean"})

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "eastl"), ".")
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
        assert(package:has_cxxtypes("eastl::string", {includes = "eastl/string.h", configs = {languages = "cxx17"}}))
    end)