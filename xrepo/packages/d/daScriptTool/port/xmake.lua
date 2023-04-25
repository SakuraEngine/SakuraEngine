includes("xmakeDep.lua")

set_languages("c11", "cxx17")
target("daScript")
    set_kind("binary")
    set_targetdir("/bin")
    set_exceptions("cxx")
    add_deps("libdaScript")
    add_files("utils/daScript/main.cpp")