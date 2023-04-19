package("wasm3")
    set_homepage("https://github.com/wasm3/wasm3")
    set_description("A fast WebAssembly interpreter and the most universal WASM runtime")
    set_license("MIT license")

    add_versions("0.5.0-skr", "396b0e861424f23175f43803d2dbcb6de59de3387265268ed36c5a6bfba3d48d")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "wasm3"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)