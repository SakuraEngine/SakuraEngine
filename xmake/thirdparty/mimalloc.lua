mimalloc_includes_dir = "$(projectdir)/thirdparty/mimalloc"

target("mimalloc")
    -- version("0.9.2alpha")
    set_group("00.thirdparty")
    set_optimize("fastest")
    add_files("$(projectdir)/thirdparty/mimalloc/build.mimalloc.c")
    add_includedirs(mimalloc_includes_dir, {public = true})
    if (is_os("windows")) then 
        add_syslinks("psapi", "shell32", "user32", "advapi32", "bcrypt")
        add_defines("_CRT_SECURE_NO_WARNINGS")
    end

    set_kind("shared")
    add_defines("MI_SHARED_LIB", {public = true})
    add_defines("MI_SHARED_LIB_EXPORT", "MI_XMALLOC=1", "MI_WIN_NOREDIRECT")