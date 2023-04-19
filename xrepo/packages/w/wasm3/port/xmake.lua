local wasm3_include_dir = "wasm3/source"
local wasm3_source_dir = "wasm3/source"

target("wasm3")
    set_kind("static")
    set_optimize("fastest")
    add_headerfiles(wasm3_include_dir.."/(**.h)", {prefixdir = "wasm3"})
    add_includedirs(wasm3_include_dir, {public = true})
    add_files(wasm3_source_dir.."/**.c")