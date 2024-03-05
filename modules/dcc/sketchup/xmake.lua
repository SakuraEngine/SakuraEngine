shared_module("SkrSketchUp", "SKR_SKETCH_UP", engine_version)
    set_exceptions("no-cxx")
    add_deps("SkrBase", {public = true})
    public_dependency("SkrCore", engine_version)
    add_includedirs("include", {public = true})
    add_includedirs("SketchUp", {public = true})
    add_files("src/build.*.cpp")
    add_rules("utils.install-libs", { libnames = {"sketchup-sdk-v2023.1.315"} })
    add_linkdirs("$(buildir)/$(os)/$(arch)/$(mode)", {public=true})
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