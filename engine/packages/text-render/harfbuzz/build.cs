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
                    .IncludeDirs(Visibility.Public, "harfbuzz")
                    .AddCppFiles(
                        "harfbuzz/hb-aat-layout.cc",
                        "harfbuzz/hb-aat-map.cc",
                        "harfbuzz/hb-blob.cc",
                        "harfbuzz/hb-buffer-serialize.cc",
                        "harfbuzz/hb-buffer.cc",
                        "harfbuzz/hb-buffer-verify.cc",
                        "harfbuzz/hb-common.cc",
                        "harfbuzz/hb-common.cc",
                        "harfbuzz/hb-draw.cc",
                        "harfbuzz/hb-face.cc",
                        "harfbuzz/hb-fallback-shape.cc",
                        "harfbuzz/hb-font.cc",
                        "harfbuzz/hb-map.cc",
                        "harfbuzz/hb-number.cc",
                        "harfbuzz/hb-ot-cff1-table.cc",
                        "harfbuzz/hb-ot-cff2-table.cc",
                        "harfbuzz/hb-ot-color.cc",
                        "harfbuzz/hb-ot-face.cc",
                        "harfbuzz/hb-ot-font.cc",
                        "harfbuzz/hb-ot-layout.cc",
                        "harfbuzz/hb-ot-map.cc",
                        "harfbuzz/hb-ot-math.cc",
                        "harfbuzz/hb-ot-meta.cc",
                        "harfbuzz/hb-ot-metrics.cc",
                        "harfbuzz/hb-ot-name.cc",
                        "harfbuzz/hb-outline.cc",

                        "harfbuzz/hb-ot-shaper-vowel-constraints.cc",
                        "harfbuzz/hb-ot-shaper-use.cc",
                        "harfbuzz/hb-ot-shaper-thai.cc",
                        "harfbuzz/hb-ot-shaper-syllabic.cc",
                        "harfbuzz/hb-ot-shaper-myanmar.cc",
                        "harfbuzz/hb-ot-shaper-khmer.cc",
                        "harfbuzz/hb-ot-shaper-indic.cc",
                        "harfbuzz/hb-ot-shaper-indic-table.cc",
                        "harfbuzz/hb-ot-shaper-hebrew.cc",
                        "harfbuzz/hb-ot-shaper-hangul.cc",
                        "harfbuzz/hb-ot-shaper-default.cc",
                        "harfbuzz/hb-ot-shaper-arabic.cc",
                        "harfbuzz/hb-ot-shape.cc",
                        "harfbuzz/hb-ot-shape-normalize.cc",
                        "harfbuzz/hb-ot-shape-fallback.cc",
                        // "port/harfbuzz/util/hb-ot-shape-closure.cc",
                        "harfbuzz/hb-ot-tag.cc",
                        "harfbuzz/hb-ot-var.cc",

                        "harfbuzz/hb-paint.cc",
                        "harfbuzz/hb-paint-extents.cc",

                        "harfbuzz/hb-set.cc",
                        "harfbuzz/hb-shape-plan.cc",
                        "harfbuzz/hb-shape.cc",
                        "harfbuzz/hb-shaper.cc",
                        "harfbuzz/hb-static.cc",
                        "harfbuzz/hb-style.cc",
                        "harfbuzz/hb-ucd.cc",
                        "harfbuzz/hb-unicode.cc"
                    )
                    // freetype integration
                    .AddCppFiles("harfbuzz/hb-ft.cc")
                    .Defines(Visibility.Private, "HAVE_FREETYPE")
                    // icu integration
                    .AddCppFiles("harfbuzz/hb-icu.cc")
                    .Defines(Visibility.Private, "HAVE_ICU", "HAVE_ICU_BUILTIN")
                    .Defines(Visibility.Private, "HAVE_OT", "HB_NO_MT")
                    .Clang_CXFlags(Visibility.Public, "-Wno-nontrivial-memcall")
                    .MSVC_CXFlags(Visibility.Private, "/wd5105");
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