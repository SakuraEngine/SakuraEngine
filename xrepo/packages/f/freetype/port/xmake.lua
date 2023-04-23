add_requires("zlib =1.2.8-skr", {system = false})

set_languages("c11", "cxx17")
add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")

target("freetype")
    set_kind("static")
    set_optimize("fastest")
    add_rules("c++.unity_build", {batchsize = 32})
    add_includedirs("freetype/include", {public=true})
    add_headerfiles("freetype/include/(**.h)")
    add_files(
        "freetype/src/autofit/autofit.c",
        "freetype/src/base/ftbase.c",
        "freetype/src/base/ftbbox.c",
        "freetype/src/base/ftbdf.c",
        "freetype/src/base/ftbitmap.c",
        "freetype/src/base/ftcid.c",
        "freetype/src/base/ftfstype.c",
        "freetype/src/base/ftgasp.c",
        "freetype/src/base/ftglyph.c",
        "freetype/src/base/ftgxval.c",
        "freetype/src/base/ftinit.c",
        "freetype/src/base/ftmm.c",
        "freetype/src/base/ftotval.c",
        "freetype/src/base/ftpatent.c",
        "freetype/src/base/ftpfr.c",
        "freetype/src/base/ftstroke.c",
        "freetype/src/base/ftsynth.c",
        "freetype/src/base/fttype1.c",
        "freetype/src/base/ftwinfnt.c",
        "freetype/src/bdf/bdf.c",
        "freetype/src/bzip2/ftbzip2.c",
        "freetype/src/cache/ftcache.c",
        "freetype/src/cff/cff.c",
        "freetype/src/cid/type1cid.c",
        "freetype/src/gzip/ftgzip.c",
        "freetype/src/lzw/ftlzw.c",
        "freetype/src/pcf/pcf.c",
        "freetype/src/pfr/pfr.c",
        "freetype/src/psaux/psaux.c",
        "freetype/src/pshinter/pshinter.c",
        "freetype/src/psnames/psnames.c",
        "freetype/src/raster/raster.c",
        "freetype/src/sdf/sdf.c",
        "freetype/src/sfnt/sfnt.c",
        "freetype/src/smooth/smooth.c",
        "freetype/src/svg/svg.c",
        "freetype/src/truetype/truetype.c",
        "freetype/src/type1/type1.c",
        "freetype/src/type42/type42.c",
        "freetype/src/winfonts/winfnt.c"
    )
    -- use skr zlib
    add_packages("zlib", {public=true})
    add_includedirs("ft-zlib", {public=false})
    add_defines("FT_CONFIG_OPTION_SYSTEM_ZLIB", {public=false})

    if (is_plat("windows")) then
        add_files("freetype/builds/windows/ftsystem.c", "freetype/builds/windows/ftdebug.c")
    else
        add_files("freetype/src/base/ftsystem.c", "freetype/src/base/ftdebug.c")
    end
    add_defines("FT2_BUILD_LIBRARY", {public=false})

    if (is_plat("windows")) then
        add_cxflags("/wd4267", "/wd4244", "/source-charset:utf-8", {public=false})
    end