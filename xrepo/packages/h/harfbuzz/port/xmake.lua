add_requires("freetype skr", {system = false})
add_requires("icu skr", {system = false})

set_languages("c11", "cxx17")

target("harfbuzz")
    set_kind("static")
    set_optimize("fastest")
    add_headerfiles("harfbuzz/src/(**.h)", "harfbuzz/src/(**.hh)")
    add_includedirs("harfbuzz/src", {public=true})
    add_packages("freetype", "icu", {public=true})
    add_files(
        "harfbuzz/src/hb-aat-layout.cc",
        "harfbuzz/src/hb-aat-map.cc",
        "harfbuzz/src/hb-blob.cc",
        "harfbuzz/src/hb-buffer-serialize.cc",
        "harfbuzz/src/hb-buffer.cc",
        "harfbuzz/src/hb-buffer-verify.cc",
        "harfbuzz/src/hb-common.cc",
        "harfbuzz/src/hb-common.cc",
        "harfbuzz/src/hb-draw.cc",
        "harfbuzz/src/hb-face.cc",
        "harfbuzz/src/hb-fallback-shape.cc",
        "harfbuzz/src/hb-font.cc",
        "harfbuzz/src/hb-map.cc",
        "harfbuzz/src/hb-number.cc",
        "harfbuzz/src/hb-ot-cff1-table.cc",
        "harfbuzz/src/hb-ot-cff2-table.cc",
        "harfbuzz/src/hb-ot-color.cc",
        "harfbuzz/src/hb-ot-face.cc",
        "harfbuzz/src/hb-ot-font.cc",
        "harfbuzz/src/hb-ot-layout.cc",
        "harfbuzz/src/hb-ot-map.cc",
        "harfbuzz/src/hb-ot-math.cc",
        "harfbuzz/src/hb-ot-meta.cc",
        "harfbuzz/src/hb-ot-metrics.cc",
        "harfbuzz/src/hb-ot-name.cc",
        "harfbuzz/src/hb-outline.cc",

        "harfbuzz/src/hb-ot-shaper-vowel-constraints.cc",
        "harfbuzz/src/hb-ot-shaper-use.cc",
        "harfbuzz/src/hb-ot-shaper-thai.cc",
        "harfbuzz/src/hb-ot-shaper-syllabic.cc",
        "harfbuzz/src/hb-ot-shaper-myanmar.cc",
        "harfbuzz/src/hb-ot-shaper-khmer.cc",
        "harfbuzz/src/hb-ot-shaper-indic.cc",
        "harfbuzz/src/hb-ot-shaper-indic-table.cc",
        "harfbuzz/src/hb-ot-shaper-hebrew.cc",
        "harfbuzz/src/hb-ot-shaper-hangul.cc",
        "harfbuzz/src/hb-ot-shaper-default.cc",
        "harfbuzz/src/hb-ot-shaper-arabic.cc",
        "harfbuzz/src/hb-ot-shape.cc",
        "harfbuzz/src/hb-ot-shape-normalize.cc",
        "harfbuzz/src/hb-ot-shape-fallback.cc",
        -- "harfbuzz/util/hb-ot-shape-closure.cc",
        "harfbuzz/src/hb-ot-tag.cc",
        "harfbuzz/src/hb-ot-var.cc",

        "harfbuzz/src/hb-paint.cc",
        "harfbuzz/src/hb-paint-extents.cc",

        "harfbuzz/src/hb-set.cc",
        "harfbuzz/src/hb-shape-plan.cc",
        "harfbuzz/src/hb-shape.cc",
        "harfbuzz/src/hb-shaper.cc",
        "harfbuzz/src/hb-static.cc",
        "harfbuzz/src/hb-style.cc",
        "harfbuzz/src/hb-ucd.cc",
        "harfbuzz/src/hb-unicode.cc"
    )
    -- freetype integration
    add_files("harfbuzz/src/hb-ft.cc")
    add_defines("HAVE_FREETYPE", {public=false})
    -- icu integration
    add_files("harfbuzz/src/hb-icu.cc")
    add_defines("HAVE_ICU", "HAVE_ICU_BUILTIN", {public=false})
    -- CoreText integration
    if (is_plat("macosx")) then
        print("harfbuzz: use macosx CoreText in ApplicationServices framework!")

        add_defines("HAVE_CORETEXT", {public=false})
        add_files("harfbuzz/src/hb-coretext.cc")
        add_frameworks("ApplicationServices", {public=false})
    elseif (is_plat("iphoneos")) then
        print("harfbuzz: use iphoneos CoreText framework!")

        add_defines("HAVE_CORETEXT", {public=false})
        add_files("harfbuzz/src/hb-coretext.cc")
        add_frameworks("CoreGraphics", "CoreText", {public=false})
    end
    --
    add_defines("HAVE_OT", {public=false})
    add_defines("HB_NO_MT", {public=false})
    if (is_plat("windows")) then
        add_cxflags("/wd4267", "/wd4244", "/source-charset:utf-8", {public=false})
    end