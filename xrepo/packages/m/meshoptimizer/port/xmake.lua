set_languages("c11", "cxx17")
add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")

target("meshoptimizer")
    set_kind("static")
    add_files("src/**.cpp")
    add_headerfiles("src/*.h", {prefixdir = "MeshOpt"})
    if is_kind("shared") and is_os("windows") then
        add_defines("MESHOPTIMIZER_API=__declspec(dllexport)")
        add_defines("MESHOPTIMIZER_API=__declspec(dllimport)", {public = true})
    end