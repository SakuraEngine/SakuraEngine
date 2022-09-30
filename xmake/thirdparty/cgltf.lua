cgltf_include_dir = "$(projectdir)/thirdparty/cgltf/include"
cgltf_source_dir = "$(projectdir)/thirdparty/cgltf/src"

target("cgltf")
    set_group("00.thirdparty")
    set_kind("static")
    add_files(cgltf_source_dir.."/cgltf.c")
    add_includedirs(cgltf_include_dir, {public = true})
    add_cxflags(project_cxflags, {public = true})