lmdb_include_dir = "$(projectdir)/thirdparty/lmdb/include"
lmdb_include_dir_private = "$(projectdir)/thirdparty/lmdb/include/lmdb"
lmdb_source_dir = "$(projectdir)/thirdparty/lmdb"

target("lmdb")
    set_group("00.thirdparty")
    set_kind("static")
    --set_optimize("fastest")
    add_files(lmdb_source_dir.."/*.c")
    add_includedirs(lmdb_include_dir, {public = true})
    add_includedirs(lmdb_include_dir_private, {public = false})