using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class Freetype
{
    static Freetype()
    {
        BuildSystem.Package("freetype")
            .AddTarget("freetype", (Target Target, PackageConfig Config) =>
            {
                if (Config.Version != new Version(2, 13, 0))
                    throw new TaskFatalError("freetype version mismatch!", "freetype version mismatch, only v2.13.0 is supported in source.");

                Target.TargetType(TargetType.Static)
                    .EnableUnityBuild()
                    .OptimizationLevel(OptimizationLevel.Fastest)
                    .IncludeDirs(Visibility.Public, "port/freetype/include")
                    .IncludeDirs(Visibility.Private, "port/ft-zlib")
                    .Defines(Visibility.Private, "FT2_BUILD_LIBRARY", "FT_CONFIG_OPTION_SYSTEM_ZLIB")
                    .Require("zlib", new PackageConfig { Version = new(1, 2, 8) })
                    .Depend(Visibility.Public, "zlib@zlib")
                    .ClangCl_CFlags(Visibility.Private, "-Wno-macro-redefined")
                    .Cl_CFlags(Visibility.Private, "/wd4005")
                    .AddCFiles(
                        "port/freetype/src/autofit/autofit.c",
                        "port/freetype/src/base/ftbase.c",
                        "port/freetype/src/base/ftbbox.c",
                        "port/freetype/src/base/ftbdf.c",
                        "port/freetype/src/base/ftbitmap.c",
                        "port/freetype/src/base/ftcid.c",
                        "port/freetype/src/base/ftfstype.c",
                        "port/freetype/src/base/ftgasp.c",
                        "port/freetype/src/base/ftglyph.c",
                        "port/freetype/src/base/ftgxval.c",
                        "port/freetype/src/base/ftinit.c",
                        "port/freetype/src/base/ftmm.c",
                        "port/freetype/src/base/ftotval.c",
                        "port/freetype/src/base/ftpatent.c",
                        "port/freetype/src/base/ftpfr.c",
                        "port/freetype/src/base/ftstroke.c",
                        "port/freetype/src/base/ftsynth.c",
                        "port/freetype/src/base/fttype1.c",
                        "port/freetype/src/base/ftwinfnt.c",
                        "port/freetype/src/bdf/bdf.c",
                        "port/freetype/src/bzip2/ftbzip2.c",
                        "port/freetype/src/cache/ftcache.c",
                        "port/freetype/src/cff/cff.c",
                        "port/freetype/src/cid/type1cid.c",
                        "port/freetype/src/gzip/ftgzip.c",
                        "port/freetype/src/lzw/ftlzw.c",
                        "port/freetype/src/pcf/pcf.c",
                        "port/freetype/src/pfr/pfr.c",
                        "port/freetype/src/psaux/psaux.c",
                        "port/freetype/src/pshinter/pshinter.c",
                        "port/freetype/src/psnames/psnames.c",
                        "port/freetype/src/raster/raster.c",
                        "port/freetype/src/sdf/sdf.c",
                        "port/freetype/src/sfnt/sfnt.c",
                        "port/freetype/src/smooth/smooth.c",
                        "port/freetype/src/svg/svg.c",
                        "port/freetype/src/truetype/truetype.c",
                        "port/freetype/src/type1/type1.c",
                        "port/freetype/src/type42/type42.c",
                        "port/freetype/src/winfonts/winfnt.c"
                    );

                if (BuildSystem.TargetOS == OSPlatform.Windows)
                {
                    Target
                        .AddCFiles("port/freetype/builds/windows/ftsystem.c", "port/freetype/builds/windows/ftdebug.c")
                        .CXFlags(Visibility.Private, "/wd4267", "/wd4244", "/utf-8")
                        .Defines(Visibility.Private, "_CRT_SECURE_NO_WARNINGS");
                }
                else
                {
                    Target
                        .AddCFiles("port/freetype/src/base/ftsystem.c", "port/freetype/src/base/ftdebug.c");
                }
            });
    }
}