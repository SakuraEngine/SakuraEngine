simdjson_include_dir = "$(projectdir)/thirdparty/simdjson/include"
simdjson_source_dir = "$(projectdir)/thirdparty/simdjson/source"

table.insert(include_dir_list, simdjson_include_dir)
table.insert(deps_list, "simdjson")

target("simdjson")
    set_kind("static")
    add_files(simdjson_source_dir.."/simdjson.cpp")
    add_includedirs(simdjson_include_dir)
    add_defines("_HAS_EXCEPTIONS=0")
    add_cxflags(project_cxflags, {public = true})