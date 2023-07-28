set_languages("c11", "cxx17")
add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")

target("tinygltf")
    set_kind("static")
    add_files("src/**.cc")
    add_headerfiles("src/*.h", {prefixdir = "tinygltf"})
    add_headerfiles("src/extern/*.h", {prefixdir = "tinygltf/extern"})
    add_headerfiles("src/extern/*.hpp", {prefixdir = "tinygltf/extern"})