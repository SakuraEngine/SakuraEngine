if (os.host() == "windows") then
add_requires("wasm3")

shared_module("SkrWASM", "SKR_WASM", engine_version)
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public = true})
    add_files("src/build.*.c", "src/build.*.cpp")
    add_packages("wasm3", {public = true})
end