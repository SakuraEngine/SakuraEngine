if (os.host() == "windows") then
    target("SkrWASM")
        set_kind("shared")
        add_deps("SkrRT")
        add_includedirs("include", {public=true})
        add_defines("SKR_WASM_SHARED", {public=true})
        add_defines("SKR_WASM_IMPL")
        add_files("src/build.*.c", "src/build.*.cpp")
end