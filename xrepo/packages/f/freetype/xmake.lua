package("freetype")
    set_homepage("https://freetype.org/")
    set_description("FreeType is a freely available software library to render fonts.")
    set_license("The FreeType License (FTL) is a BSD-style license with a credit clause and thus compatible with the GNU Public License (GPL) version 3, but not with the GPL version 2.")
    
    add_versions("2.13.0-skr", "0986ddbdaaac65da525716e7134038edcc7d8d1ca965629717231a89f32b839f")

    add_deps("zlib =1.2.8-skr")
    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "freetype"), ".")
        os.cp(path.join(package:scriptdir(), "port", "ft-zlib"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)

    on_test(function (package)
        assert(package:has_cfuncs("FT_Init_FreeType", {includes = {"ft2build.h", "freetype/freetype.h"}}))
    end)
