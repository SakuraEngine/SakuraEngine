fmt_include_dir = "$(projectdir)/thirdparty/fmt/include"
fmt_source_dir = "$(projectdir)/thirdparty/fmt/src"

target("fmt")
    set_kind("static")
    add_files(fmt_source_dir.."/format.cc", fmt_source_dir.."/os.cc")
    add_includedirs(fmt_include_dir, {public = true})
    add_cxflags(project_cxflags, {public = true})