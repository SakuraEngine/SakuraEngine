package("v8")
    set_homepage("https://chromium.googlesource.com/v8/v8.git")
    set_description("V8 JavaScript Engine")
    set_urls("https://github.com/SakuraEngine/v8-compile/releases/download/v8_$(version)/",
    {
        version = function (version) 
            return version:gsub("(%S*)-skr", "%1")
        end
    })
    add_versions("12.4-skr", "be9e9cbd9f6874e455a537bdefdcc24f895bc4d2b2646f5e25217436dcf6c081")

    -- v8 sys env
    if is_plat("linux", "bsd") then
        add_syslinks("pthread", "dl")
    elseif is_plat("windows") then
        add_syslinks("user32", "winmm", "advapi32", "dbghelp", "shlwapi")
    end

    -- v8 env
    add_links(
        "v8.dll",
        "v8_libbase.dll",
        "v8_libplatform.dll",
        "third_party_zlib.dll")

    -- v8 def
    add_defines("USING_V8_PLATFORM_SHARED", { public = true })
    add_defines("USING_V8_SHARED", { public = true })

    on_download(function (package, opt) 
        import("net.http")
        import("utils.archive")

        -- get url config
        local raw_url = opt.url
        local plat = package:plat()
        local arch = package:arch()

        -- combine url
        local url = format("%sv8-%s-%s.tgz", raw_url, plat, arch)

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
        os.cp("lib/*.lib", package:installdir("lib"))
        os.cp("bin/*.dll", package:installdir("bin"))
        os.cp("bin/*.pdb", package:installdir("bin"))
    end)

    on_test(function (package) 
        assert(package:has_cxxfuncs("v8::V8::InitializePlatform(v8::platform::NewDefaultPlatform().get())", {
            configs = {
                languages = "c++17"
            }, 
            includes = { 
                "v8.h", 
                "libplatform/libplatform.h" 
            }
        }))
    end)