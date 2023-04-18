package("zlib")
    set_homepage("http://zlib.net/")
    set_description("zlib 1.2.13 is a general purpose data compression library.")
    set_license("ZLIB License")
    
    add_versions("skr", "e3ed8a269df53e9a8530057f4949f48885f746ae0fe9d72e5b4c616833494dc2")

    add_includedirs("zlib/include", {public=true})
    if is_plat("windows")then
        add_links("zlib/lib/windows/x64/zlibstatic", {public=true})
    elseif is_plat("macosx")then
        add_links("zlib/lib/macos/x86_64/z", {public=true})
    else
        print("not supported platform")
    end

    on_install(function (package)
        local zlib_dir = package:installdir(".")
        os.cp(path.join(package:scriptdir(), "port", "zlib"), zlib_dir)
        -- local configs = {}
        -- import("package.tools.xmake").install(package, configs)
    end)