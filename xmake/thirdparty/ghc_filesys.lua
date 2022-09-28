ghc_fs_include_dir = "$(projectdir)/thirdparty/ghc_fs/include"
ghc_fs_source_dir = "$(projectdir)/thirdparty/ghc_fs"

target("ghc_fs")
    set_kind("static")
    add_files(ghc_fs_source_dir.."/filesystem.cpp")
    if is_os("windows") then
        add_links("shell32")
    end
    add_includedirs(ghc_fs_include_dir, {public = true})
    add_cxflags(project_cxflags, {public = true})
