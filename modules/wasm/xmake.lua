if (os.host() == "windows") then
    target("SkrWASM")
        add_rules("skr.module", {api = "SKR_WASM"})
        add_deps("SkrRT")
        add_includedirs("include", {public=true})
        add_files("src/build.*.c", "src/build.*.cpp")
end