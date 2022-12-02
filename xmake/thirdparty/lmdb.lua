lmdb_include_dir = "$(projectdir)/thirdparty/lmdb/include"
lmdb_include_dir_private = "$(projectdir)/thirdparty/lmdb/include/lmdb"
lmdb_source_dir = "$(projectdir)/thirdparty/lmdb"

target("lmdb")
    set_group("00.thirdparty")
    add_includedirs(lmdb_include_dir, {public = true})
    add_includedirs(lmdb_include_dir_private, {public = false})
    if (is_os("windows")) then 
        set_kind("headeronly")
        add_links(sdk_libs_dir.."lmdb", {public=true} )
    else
        set_kind("static")
        set_optimize("fastest")
        add_files(lmdb_source_dir.."/*.c")
    end