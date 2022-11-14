fmt_include_dir = "$(projectdir)/thirdparty/fmt/include"
fmt_source_dir = "$(projectdir)/thirdparty/fmt/src"

target("fmt")
    set_group("00.thirdparty")
    set_kind("static")
    set_exceptions("no-cxx")
    add_defines("_HAS_EXCEPTIONS=0")
    add_defines("FMT_EXCEPTIONS=0", {public = true})
    add_files(fmt_source_dir.."/format.cc", fmt_source_dir.."/os.cc")
    add_includedirs(fmt_include_dir, {public = true})
    add_cxflags(project_cxflags, {public = true})