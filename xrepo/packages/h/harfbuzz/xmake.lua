package("harfbuzz")
    set_homepage("https://github.com/harfbuzz/harfbuzz/")
    set_description("HarfBuzz is a text shaping engine. It primarily supports OpenType, but also Apple Advanced Typography.")
    set_license("HarfBuzz is licensed under the so-called 'Old MIT' license. Details follow: https://github.com/harfbuzz/harfbuzz/blob/main/COPYING")
    
    add_versions("7.1.0-skr", "7ca8368e3fb5b1a1066b227ed4d1c15dfa364617ab8bbabea929dec8f3c6a739")

    add_deps("freetype =2.13.0-skr")
    add_deps("icu =72.1.0-skr")
    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "harfbuzz"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)