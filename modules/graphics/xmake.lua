shared_module("SkrGraphics", "SKR_GRAPHICS", engine_version)
    set_group("01.modules")
    set_exceptions("no-cxx")
    add_deps("vulkan", "SkrBase", {public = true})
    public_dependency("SkrCore", engine_version)
    add_includedirs("include", {public = true})
    add_files("src/build.*.c", "src/build.*.cpp")
    -- install SDKs
    libs_to_install = {}
    if(os.host() == "windows") then
        table.insert(libs_to_install, "dstorage-1.2.1")    
        table.insert(libs_to_install, "dxc")
        table.insert(libs_to_install, "amdags")
        table.insert(libs_to_install, "nvapi")
        table.insert(libs_to_install, "nsight")
        table.insert(libs_to_install, "WinPixEventRuntime")
    end
    add_rules("utils.install-libs", { libnames = libs_to_install })
    
    if (is_os("windows")) then 
        add_linkdirs("$(buildir)/$(os)/$(arch)/$(mode)", {public=true})
        add_links("nvapi_x64", {public = true})
        add_links("WinPixEventRuntime", {public = true})
    end