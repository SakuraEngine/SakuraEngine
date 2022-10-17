target("UsdCore")
    set_group("02.tools")
        
    if(has_config("is_msvc")) then
        add_cxflags("/EHsc")
        add_cxxflags("/EHsc")
        add_defines("_HAS_EXCEPTIONS=1")
    elseif(has_config("is_clang")) then
        add_cxflags("-fexceptions", "-fcxx-exceptions")
        add_cxxflags("-fexceptions", "-fcxx-exceptions")
    end

    add_rules("skr.module", {api = "USDCORE"})
    add_rules("c++.codegen", {
        files = {"include/**.h", "include/**.hpp"},
        rootdir = "include/"
    })
    add_includedirs("include",{public=true})
    add_deps("SkrTool", "GameRT")
    add_files("src/**.cpp")
    -- TODO: make these private
    add_includedirs("thirdparty", "thirdparty/python3", {public=true})
    add_defines("__TBB_NO_IMPLICIT_LINKAGE=1", {public=true})
    add_defines("BOOST_PYTHON_NO_LIB=1", {public=true})
    add_defines("Py_NO_ENABLE_SHARED=1", {public=true})
    before_build(function(target)
        import("core.base.scheduler")
        local function upzip_tasks(targetname)
            import("core.project.task")
            task.run("unzip-usd")
        end
        scheduler.co_start(upzip_tasks, targetname)
    end)
    add_links("ar", "arch", "gf", "js", "kind", "ndr", "pcp", "plug", "sdf", "sdr", "tf", "trace", {public=true})
    add_links("usd", "usdGeom", "usdHydra", "usdLux", "usdMedia", "usdPhysics", "usdRender", "usdRi", "usdShade", "usdUtils", {public=true})
    add_links("vt", "work", {public=true})