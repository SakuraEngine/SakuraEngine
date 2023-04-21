set_languages("c11", "cxx17")
option("runtime_shared", { default = true, showmenu = true })

target("eastl")
    set_kind("static")
    set_optimize("fastest")
    add_headerfiles("EASTL/(**.h)")
    add_headerfiles("EASTL/(**.hpp)")
    add_includedirs("EASTL", {public = true})
    add_files("EASTL/**.cpp")
    if not has_config("runtime_shared") then
        add_defines("RUNTIME_ALL_STATIC")
    end