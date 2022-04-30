mimalloc_sources_dir = "$(projectdir)/thirdparty/mimalloc"
mimalloc_includes_dir = "$(projectdir)/thirdparty/mimalloc/include"

table.insert(source_list, mimalloc_sources_dir.."/unitybuild.c")
table.insert(include_dir_list, mimalloc_includes_dir)