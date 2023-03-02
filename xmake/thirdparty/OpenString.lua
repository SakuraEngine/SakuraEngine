ostring_include_dir = "$(projectdir)/thirdparty/OpenString/include"
ostring_private_include_dir = "$(projectdir)/thirdparty/OpenString/include/OpenString"
ostring_source_dir = "$(projectdir)/thirdparty/OpenString/source"

table.insert(source_list, ostring_source_dir.."/*.cpp")
table.insert(include_dir_list, ostring_include_dir)
table.insert(private_include_dir_list, ostring_private_include_dir)
table.insert(defs_list, "OPEN_STRING_NS=skr::text")
table.insert(defs_list, "OPEN_STRING_API=RUNTIME_API")