package("v8")
    set_homepage("https://chromium.googlesource.com/v8/v8.git")
    set_description("V8 JavaScript Engine")
    set_urls("https://github.com/SakuraEngine/v8-compile/releases/download/$(version)/",
    {
        version = function (version) 
            return version:gsub("(%S*)-skr", "%1")
        end
    })
    add_versions("11.2-test-skr", "65057ad701861ce61dcceebfaff9b717861e8c54b61f5cc3c7194e9a4f4895e6")
    add_versions("11.2-skr", "b1cab859d7eb654527fc9aaec81ef2942424d3ff69cb9a20a21a2021696bb644")

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
        import("core.base.json")

        -- get url config
        local base_url = opt.url
        local plat = package:plat()
        local arch = package:arch()
        local toolchain = "unknown"
        if package:is_plat("windows") then
            if package:toolchain("clang-cl") then
                toolchain = "clang-cl"
            elseif package:toolchain("msvc") then
                toolchain = "msvc"
            else
                toolchain = "msvc"
            end
        end 

        -- combine url
        local manifest_url = base_url.."manifest.json"

        -- get download config
        local sourcedir = opt.sourcedir
        local manifest_file = "manifest.json"
        local manifest_hash = package:sourcehash(opt.url_alias)

        -- download manifest file
        if not os.isfile(manifest_file) or sourcehash ~= hash.sha256(manifest_file) then
            -- attempt to remove manifest file first
            print("downloading manifest file from \"%s\"\n", manifest_url)
            os.tryrm(manifest_file)
            http.download(manifest_url, manifest_file)

            -- check hash
            if manifest_hash and manifest_hash ~= hash.sha256(manifest_file) then
                raise("unmatched checksum, current hash(%s) != original hash(%s)"
                    , hash.sha256(manifest_file):sub(1, 8)
                    , manifest_hash:sub(1, 8))
            end
        end

        -- load manifest and solve package url
        local manifest = json.loadfile(manifest_file)
        local package_file = format("%s-%s-%s.tgz", plat, arch, toolchain)
        local package_url = base_url..package_file
        local package_hash = manifest[package_file]
        if not package_hash then 
            raise("package(%s) not found in manifest file(%s)\n", package_file, manifest_file)
        end

        -- process cache and try download package file
        local cached = true
        if not os.isfile(package_file) or package_hash ~= hash.sha256(package_file) then
            cached = false

            -- attempt to remove package file first
            print("downloading package file from \"%s\"\n", package_url)
            os.tryrm(package_file)
            http.download(package_url, package_file)

            -- check hash
            if package_hash and package_hash ~= hash.sha256(package_file) then
                raise("unmatched checksum, current hash(%s) != original hash(%s)", hash.sha256(package_file):sub(1, 8), package_hash:sub(1, 8))
            end
        end

        -- extract package file
        local sourcedir_tmp = sourcedir .. ".tmp"
        os.rm(sourcedir_tmp)
        if archive.extract(package_file, sourcedir_tmp) then
            os.rm(sourcedir)
            os.mv(sourcedir_tmp, sourcedir)
        else
            -- if it is not archive file, we need only create empty source file and use package:originfile()
            os.tryrm(sourcedir)
            os.mkdir(sourcedir)
        end

        -- save original file path
        package:originfile_set(path.absolute(package_file))
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