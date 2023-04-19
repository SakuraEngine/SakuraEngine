local lmdb_include_dir = "lmdb/include"
local lmdb_include_dir_private = "lmdb/include/lmdb"
local lmdb_source_dir = "lmdb"

target("lmdb")
    set_kind("static")
    set_optimize("fastest")
    add_headerfiles(lmdb_include_dir.."/(**.h)")
    add_includedirs(lmdb_include_dir, {public = true})
    add_includedirs(lmdb_include_dir_private, {public = false})
    add_files(lmdb_source_dir.."/*.c")