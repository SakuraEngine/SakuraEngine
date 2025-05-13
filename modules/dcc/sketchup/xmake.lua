target("SkrSketchUpCommon")
    set_kind("phony")
    set_exceptions("no-cxx")
    public_dependency("SkrCore", engine_version)
    add_includedirs("SketchUp", {public = true})
    add_linkdirs("$(builddir)/$(os)/$(arch)/$(mode)", {public=true})
    -- install
    skr_install_rule()
    skr_install("download", {
        name = "sketchup-sdk-v2023.1.315",
        install_func = "sdk",
    })

shared_module("SkrSketchUp", "SKR_SKETCH_UP", engine_version)
    set_exceptions("no-cxx")
    add_deps("SkrSketchUpCommon", {public = true})
    add_files("src/build.*.cpp")
    if is_os("windows") then 
        add_links("SketchUpAPI", {public = true})
    elseif is_os("macosx") then
        add_links(
            "CommonGeometry",
            "CommonGeoutils",
            "CommonImage",
            "CommonPreferences",
            "CommonUnits",
            "CommonUtils",
            "CommonZip"
        , {public = true})
    end

shared_module("SkrSketchUpLive", "SKR_SKETCH_UP", engine_version)
    set_exceptions("no-cxx")
    add_deps("SkrSketchUpCommon", {public = true})
    add_files("src/build.*.cpp")
    add_linkdirs("libs/$(os)", {public = true})
    add_links("sketchup", {public = true})
    add_links("x64-msvcrt-ruby270", {public = true})