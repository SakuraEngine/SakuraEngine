cgltf_include_dir = "$(projectdir)/thirdparty/cgltf/include"
cgltf_source_dir = "$(projectdir)/thirdparty/cgltf/src"

target("cgltf")
    set_group("00.thirdparty")
    set_kind("headeronly")
    add_includedirs(cgltf_include_dir, {public = true})
    add_cxflags(project_cxflags, {public = true})
    add_defines(defs_list, {public = true})
    if (is_os("windows") and not is_mode("asan")) then 
        add_links(sdk_libs_dir.."cgltf", {public=true} )
    else
        set_kind("static")
        add_files(cgltf_source_dir.."/cgltf.c")
    end