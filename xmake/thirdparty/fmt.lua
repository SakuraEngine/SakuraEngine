fmt_include_dir = "$(projectdir)/thirdparty/fmt/include"
fmt_source_dir = "$(projectdir)/thirdparty/fmt/src"

target("fmt")
    set_group("00.thirdparty")
    set_exceptions("no-cxx")
    add_defines("_HAS_EXCEPTIONS=0")
    add_defines("FMT_EXCEPTIONS=0", {public = true})
    add_includedirs(fmt_include_dir, {public = true})
    add_cxflags(project_cxflags, {public = true})
    if (is_os("windows") and not is_mode("asan")) then 
        set_kind("headeronly")
        add_linkdirs(sdk_libs_dir, {public=true})
        if (is_mode("release")) then
            add_links("fmt", {public=true} )
        else
            add_links("fmtd", {public=true} )
        end
    else
        set_kind("static")
        add_files(fmt_source_dir.."/format.cc", fmt_source_dir.."/os.cc")
    end