simdjson_sources_dir = "$(projectdir)/thirdparty/simdjson/source"
simdjson_includes_dir = "$(projectdir)/thirdparty/simdjson/include"

target("simdjson")
    set_kind("static")
    add_files(simdjson_sources_dir.."/*.cpp")
    add_includedirs(simdjson_includes_dir, {public = true})