if (os.host() == "windows") then
    target("SkrWASM")
        set_group("01.modules")
        add_rules("skr.module", {api = "SKR_WASM", version = engine_version})
        public_dependency("SkrRT", engine_version)
        add_includedirs("include", {public=true})
        add_files("src/build.*.c", "src/build.*.cpp")
end