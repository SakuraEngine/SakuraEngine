target("UsdCore")
    set_group("02.tools")
    add_rules("skr.shared", {api = "USDCORE"})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/", disable_meta = true
    })
    public_dependency("SkrToolCore", engine_version)
    add_includedirs("include",{public=true})
    add_files("src/**.cpp")

    -- install plugin descriptors
    add_rules("utils.install-resources", {
        extensions = {".json", ".usda", ".glslfx"},
        outdir = "usd_plugins", 
        rootdir = os.curdir().."/usd_plugins"})
    add_files("usd_plugins/**.json", "usd_plugins/**.usda", "usd_plugins/**.glslfx")

    -- enable exception
    set_exceptions("cxx")

    -- unzip sdk
    add_rules("utils.install-libs", { libnames = { "usd" } })

    -- TODO: make these private
    if is_plat("windows") then
        set_runtimes("MD")
        add_links("ar", "arch", "gf", "js", "kind", "ndr", "pcp", "plug", "sdf", "sdr", "tf", "trace", {public=true})
        add_links("usd", "usdGeom", "usdHydra", "usdLux", "usdMedia", "usdPhysics", "usdRender", "usdRi", "usdShade", "usdUtils", {public=true})
        add_links("usdVol", "usdSkel", "vt", "work", {public=true})
        add_links("boost_python39-vc142-mt-x64-1_70", {public=true}) -- boost 1.70
    end
    add_includedirs("thirdparty", "thirdparty/python3", {public=true})
    add_defines("__TBB_NO_IMPLICIT_LINKAGE=1", {public=true}) -- tbb
    add_defines("BOOST_PYTHON_NO_LIB=1", {public=true}) -- boost_python
    --add_defines("BOOST_PYTHON_STATIC_LIB=0", {public=true}) -- py39
    --add_defines("Py_NO_ENABLE_SHARED=1", {public=true}) -- py39

