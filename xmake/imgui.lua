imgui_sources_dir = "$(projectdir)/thirdparty/imgui"
imgui_includes_dir = "$(projectdir)/thirdparty/imgui/include"

table.insert(include_dir_list, imgui_includes_dir)
table.insert(deps_list, "imgui")

target("imgui")
    set_kind("static")
    add_files(imgui_sources_dir.."/unitybuild.cpp")
    add_includedirs(imgui_includes_dir)
