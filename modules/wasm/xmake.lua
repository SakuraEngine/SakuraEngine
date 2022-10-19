if (os.host() == "windows") then
    shared_module("SkrWASM", "SKR_WASM", engine_version)
        set_group("01.modules")
        public_dependency("SkrRT", engine_version)
        add_includedirs("include", {public=true})
        add_files("src/build.*.c", "src/build.*.cpp")
end