local fmt_include_dir = "fmt/include"
local fmt_source_dir = "fmt/src"

set_languages("c11", "cxx17")

target("fmt")
    set_kind("static")
    set_optimize("fastest")
    set_exceptions("no-cxx")
    add_defines("_HAS_EXCEPTIONS=0")
    add_defines("FMT_EXCEPTIONS=0", {public = true})
    add_headerfiles(fmt_include_dir.."/(**.h)")
    add_includedirs(fmt_include_dir, {public = true})
    add_cxflags(project_cxflags, {public = true})
    add_files(fmt_source_dir.."/format.cc", fmt_source_dir.."/os.cc")