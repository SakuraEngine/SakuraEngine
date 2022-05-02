fmt_include_dir = "$(projectdir)/thirdparty/fmt/include"
fmt_source_dir = "$(projectdir)/thirdparty/fmt/src"

table.insert(include_dir_list, fmt_include_dir)
table.insert(deps_list, "fmt")

target("fmt")
    set_kind("static")
    add_files(fmt_source_dir.."/format.cc", fmt_source_dir.."/os.cc")
    add_includedirs(fmt_include_dir)
    add_defines("_HAS_EXCEPTIONS=0")
    add_cxflags(project_cxflags, {public = true})