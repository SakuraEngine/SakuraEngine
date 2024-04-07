package("v8")
    set_homepage("https://chromium.googlesource.com/v8/v8.git")
    set_description("V8 JavaScript Engine")
    set_urls("https://github.com/SakuraEngine/v8-compile/release/douwnload/v8_$(version)/",
    {
        version = function (version) 
            return version:gsub("(%S*)-skr", "%1")
        end
    })
    add_versions("12.4-skr", "a28d420d2755bef7c0b049e6f233a22facc9f674141df631ce2ca21adbb7b6f5")

    on_download(function (package, opt) 
        import("net.http")
        import("utils.archive")

        -- get url config
        local raw_url = opt.url
        local plat = package:plat()
        local arch = package:arch()

        -- combine url
        local url = format("%s/v8-%s-%s.tgz", raw_url, plat, arch)

        -- get download config
        local sourcedir = opt.sourcedir
        local packagefile = path.filename(url)
        local sourcehash = package:sourcehash(opt.url_alias)

        -- process cache and try download package file
        local cached = true
        if not os.isfile(packagefile) or sourcehash ~= hash.sha256(packagefile) then
            cached = false

            -- attempt to remove package file first
            os.tryrm(packagefile)
            http.download(url, packagefile)

            -- check hash
            if sourcehash and sourcehash ~= hash.sha256(packagefile) then
                raise("unmatched checksum, current hash(%s) != original hash(%s)", hash.sha256(packagefile):sub(1, 8), sourcehash:sub(1, 8))
            end
        end

        -- extract package file
        local sourcedir_tmp = sourcedir .. ".tmp"
        os.rm(sourcedir_tmp)
        if archive.extract(packagefile, sourcedir_tmp) then
            os.rm(sourcedir)
            os.mv(sourcedir_tmp, sourcedir)
        else
            -- if it is not archive file, we need only create empty source file and use package:originfile()
            os.tryrm(sourcedir)
            os.mkdir(sourcedir)
        end

        -- save original file path
        package:originfile_set(path.absolute(packagefile))
    end)

    on_install("windows|x64", function (package) 
        os.mkdir(package:installdir())
        os.cp("include", package:installdir())
        os.cp("lib", package:installdir())
        os.cp("bin", package:installdir())
    end)

    on_test(function (package) 
        assert(package:has_cxxfuncs("v8::V8::InitializePlatform(0)", {configs = {languages = "c++17"}, includes = "v8.h"}))
    end)