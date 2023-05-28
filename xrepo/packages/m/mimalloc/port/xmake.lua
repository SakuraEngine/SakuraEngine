target("mimalloc")
    -- version("0.9.2alpha")
    set_group("00.thirdparty")
    set_kind("static")
    set_optimize("fastest")
    add_files("mimalloc/build.mimalloc.c")
    add_headerfiles("mimalloc/*.h")
    add_includedirs("mimalloc", {public = true})
    if (is_os("windows")) then 
        add_syslinks("advapi32", "user32", {public = true})
    end