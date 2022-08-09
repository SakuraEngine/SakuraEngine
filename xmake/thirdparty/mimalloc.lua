mimalloc_sources_dir = "$(projectdir)/thirdparty/mimalloc"
mimalloc_includes_dir = "$(projectdir)/thirdparty/mimalloc/include"

if (true) then
    table.insert(source_list, mimalloc_sources_dir.."/unitybuild.c")
else
    table.insert(source_list, mimalloc_sources_dir.."/stats.c")
    table.insert(source_list, mimalloc_sources_dir.."/random.c")
    table.insert(source_list, mimalloc_sources_dir.."/os.c")
    table.insert(source_list, mimalloc_sources_dir.."/bitmap.c")
    table.insert(source_list, mimalloc_sources_dir.."/arena.c")
    table.insert(source_list, mimalloc_sources_dir.."/segment-cache.c")
    table.insert(source_list, mimalloc_sources_dir.."/segment.c")
    table.insert(source_list, mimalloc_sources_dir.."/page.c")
    table.insert(source_list, mimalloc_sources_dir.."/alloc.c")
    table.insert(source_list, mimalloc_sources_dir.."/alloc-aligned.c")
    table.insert(source_list, mimalloc_sources_dir.."/alloc-posix.c")
    table.insert(source_list, mimalloc_sources_dir.."/heap.c")
    table.insert(source_list, mimalloc_sources_dir.."/options.c")
    table.insert(source_list, mimalloc_sources_dir.."/init.c")
end

table.insert(include_dir_list, mimalloc_includes_dir)
