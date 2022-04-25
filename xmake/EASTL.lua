eastl_sources_dir = "$(projectdir)/thirdparty/EASTL/EASTL"
eastl_includes_dir = "$(projectdir)/thirdparty/EASTL"

table.insert(source_list, eastl_sources_dir.."/eastl.cpp")
table.insert(include_dir_list, eastl_includes_dir)