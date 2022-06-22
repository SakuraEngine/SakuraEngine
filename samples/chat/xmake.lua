
set_project("chat")
add_repositories("skr-repo https://github.com/SakuraEngine/xmake-repo")
add_requires("gamenetworkingsockets", {configs={shared=true, webrtc=true, vs_runtime="MT"}})

target("chat")
    set_languages("cxx17")
    set_kind("binary")
    add_files("main.cpp", "signal_client.cpp", "imgui_impl_sdl.cpp")
    add_deps("SkrRT", "SkrRenderer")
    add_packages("gamenetworkingsockets")
