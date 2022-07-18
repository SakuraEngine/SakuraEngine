cgltf_include_dir = "$(projectdir)/thirdparty/cgltf/include"
cgltf_source_dir = "$(projectdir)/thirdparty/cgltf/src"

table.insert(include_dir_list, cgltf_include_dir)
table.insert(deps_list, "cgltf")

target("cgltf")
    set_kind("static")
    add_files(cgltf_source_dir.."/cgltf.c")
    add_includedirs(cgltf_include_dir)
    add_cxflags(project_cxflags, {public = true})