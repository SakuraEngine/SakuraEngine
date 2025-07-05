using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class Harfbuzz
{
    static Harfbuzz()
    {
        BuildSystem.Package("harfbuzz")
            .AddTarget("harfbuzz", (Target Target, PackageConfig Config) =>
            {
                Target.TargetType(TargetType.Static)
                    .CppVersion("20")
                    .Require("freetype", new PackageConfig { Version = new Version(2, 13, 0) })
                    .Require("icu", new PackageConfig { Version = new Version(72, 1, 0) })
                    .Depend(Visibility.Public, "freetype@freetype")
                    .Depend(Visibility.Public, "icu@icu")
                    .IncludeDirs(Visibility.Public, "port/harfbuzz/src")
                    .AddCppFiles(
                        "port/harfbuzz/src/hb-aat-layout.cc",
                        "port/harfbuzz/src/hb-aat-map.cc",
                        "port/harfbuzz/src/hb-blob.cc",
                        "port/harfbuzz/src/hb-buffer-serialize.cc",
                        "port/harfbuzz/src/hb-buffer.cc",
                        "port/harfbuzz/src/hb-buffer-verify.cc",
                        "port/harfbuzz/src/hb-common.cc",
                        "port/harfbuzz/src/hb-common.cc",
                        "port/harfbuzz/src/hb-draw.cc",
                        "port/harfbuzz/src/hb-face.cc",
                        "port/harfbuzz/src/hb-fallback-shape.cc",
                        "port/harfbuzz/src/hb-font.cc",
                        "port/harfbuzz/src/hb-map.cc",
                        "port/harfbuzz/src/hb-number.cc",
                        "port/harfbuzz/src/hb-ot-cff1-table.cc",
                        "port/harfbuzz/src/hb-ot-cff2-table.cc",
                        "port/harfbuzz/src/hb-ot-color.cc",
                        "port/harfbuzz/src/hb-ot-face.cc",
                        "port/harfbuzz/src/hb-ot-font.cc",
                        "port/harfbuzz/src/hb-ot-layout.cc",
                        "port/harfbuzz/src/hb-ot-map.cc",
                        "port/harfbuzz/src/hb-ot-math.cc",
                        "port/harfbuzz/src/hb-ot-meta.cc",
                        "port/harfbuzz/src/hb-ot-metrics.cc",
                        "port/harfbuzz/src/hb-ot-name.cc",
                        "port/harfbuzz/src/hb-outline.cc",

                        "port/harfbuzz/src/hb-ot-shaper-vowel-constraints.cc",
                        "port/harfbuzz/src/hb-ot-shaper-use.cc",
                        "port/harfbuzz/src/hb-ot-shaper-thai.cc",
                        "port/harfbuzz/src/hb-ot-shaper-syllabic.cc",
                        "port/harfbuzz/src/hb-ot-shaper-myanmar.cc",
                        "port/harfbuzz/src/hb-ot-shaper-khmer.cc",
                        "port/harfbuzz/src/hb-ot-shaper-indic.cc",
                        "port/harfbuzz/src/hb-ot-shaper-indic-table.cc",
                        "port/harfbuzz/src/hb-ot-shaper-hebrew.cc",
                        "port/harfbuzz/src/hb-ot-shaper-hangul.cc",
                        "port/harfbuzz/src/hb-ot-shaper-default.cc",
                        "port/harfbuzz/src/hb-ot-shaper-arabic.cc",
                        "port/harfbuzz/src/hb-ot-shape.cc",
                        "port/harfbuzz/src/hb-ot-shape-normalize.cc",
                        "port/harfbuzz/src/hb-ot-shape-fallback.cc",
                        // "port/harfbuzz/util/hb-ot-shape-closure.cc",
                        "port/harfbuzz/src/hb-ot-tag.cc",
                        "port/harfbuzz/src/hb-ot-var.cc",

                        "port/harfbuzz/src/hb-paint.cc",
                        "port/harfbuzz/src/hb-paint-extents.cc",

                        "port/harfbuzz/src/hb-set.cc",
                        "port/harfbuzz/src/hb-shape-plan.cc",
                        "port/harfbuzz/src/hb-shape.cc",
                        "port/harfbuzz/src/hb-shaper.cc",
                        "port/harfbuzz/src/hb-static.cc",
                        "port/harfbuzz/src/hb-style.cc",
                        "port/harfbuzz/src/hb-ucd.cc",
                        "port/harfbuzz/src/hb-unicode.cc"
                    )
                    // freetype integration
                    .AddCppFiles("port/harfbuzz/src/hb-ft.cc")
                    .Defines(Visibility.Private, "HAVE_FREETYPE")
                    // icu integration
                    .AddCppFiles("port/harfbuzz/src/hb-icu.cc")
                    .Defines(Visibility.Private, "HAVE_ICU", "HAVE_ICU_BUILTIN")
                    .Defines(Visibility.Private, "HAVE_OT", "HB_NO_MT");
                    // CoreText integration
                    /*
                    if (is_plat("macosx")) then
                        print("harfbuzz: use macosx CoreText in ApplicationServices framework!")

                        add_defines("HAVE_CORETEXT", {public= false})
                        add_files("harfbuzz/src/hb-coretext.cc")
                        add_frameworks("ApplicationServices", {public= false})
                    elseif(is_plat("iphoneos")) then
                        print("harfbuzz: use iphoneos CoreText framework!")

                        add_defines("HAVE_CORETEXT", {public= false})
                        add_files("harfbuzz/src/hb-coretext.cc")
                        add_frameworks("CoreGraphics", "CoreText", {public= false})
                    end
                    */
                    if (BuildSystem.TargetOS == OSPlatform.Windows)
                    {
                        Target.CXFlags(Visibility.Private, "/wd4267", "/wd4244", "/utf-8")
                            .Defines(Visibility.Private, "_CRT_SECURE_NO_WARNINGS");
                    }
            });

    }
}