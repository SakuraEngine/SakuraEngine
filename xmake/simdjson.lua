simdjson_sources_dir = "$(projectdir)/thirdparty/simdjson/source"
simdjson_includes_dir = "$(projectdir)/thirdparty/simdjson/include"

table.insert(include_dir_list, simdjson_includes_dir)
table.insert(deps_list, "simdjson")

target("simdjson")
    set_kind("static")
    add_files(simdjson_sources_dir.."/*.cpp")
    add_includedirs(simdjson_includes_dir)