local cgltf_include_dir = "cgltf/include"
local cgltf_source_dir = "cgltf/src"

set_languages("c11", "cxx17")

target("cgltf")
    set_kind("static")
    set_optimize("fastest")
    add_headerfiles(cgltf_include_dir.."/(**.h)")
    add_includedirs(cgltf_include_dir, {public = true})
    add_cxflags(project_cxflags, {public = true})
    add_defines(defs_list, {public = true})
    add_files(cgltf_source_dir.."/cgltf.c")