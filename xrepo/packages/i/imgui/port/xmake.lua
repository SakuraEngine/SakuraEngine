set_languages("c11", "cxx17")
add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")

option("runtime_shared", { default = true, showmenu = true })

target("imgui")
    set_kind("static")
    set_optimize("fastest")
    add_files("imgui/build.*.cpp")
    add_files("cimgui/src/cimgui.cpp")
    add_includedirs("imgui", {public=false})

    if not has_config("runtime_shared") then
        add_defines("RUNTIME_ALL_STATIC")
    end

    add_includedirs("imgui/include", {public=true})
    add_includedirs("cimgui/include", {public=true})
    add_headerfiles("cimgui/include/(**.h)")
    add_headerfiles("imgui/include/(**.h)")