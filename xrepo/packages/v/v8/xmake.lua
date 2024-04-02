package("v8")
    set_homepage("https://chromium.googlesource.com/v8/v8.git")
    set_description("V8 JavaScript Engine")
    set_urls("")
    add_versions("12.4-lkgr-skr", "https://github.com/SakuraEngine/v8-compile/release/douwnload/v8$(version)/"
    {version = function (version)
        return version:gsub()
    end})

    on_install("windows|x64", function (package) 
        
    end)

    on_test(function (package) 
        
    end)