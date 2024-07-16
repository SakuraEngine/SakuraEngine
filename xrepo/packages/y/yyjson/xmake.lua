package("yyjson")
    set_homepage("https://github.com/ibireme/yyjson")
    set_description("The fastest JSON library in C.")
    set_license("MIT License")
    add_versions("v0.9.0", "f6c5cd82732691711c93a768f2ac0853e745018ad1dc1be0865bcde912b2f8af")

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "src"), ".")

        local content = [[
            set_languages("c11")
            add_rules("mode.debug", "mode.release", "mode.releasedbg", "mode.asan")

            target("yyjson")
                set_kind("static")
                set_optimize("fastest")
                add_files("src/**.c")
                add_headerfiles("src/*.h", {prefixdir = "yyjson"})
        ]]
        -- add fp fast flags
        if package:is_toolchain("msvc") then
            content = content..[[add_cxflags("/fp:fast")]]
        elseif package:is_toolchain("clang") or package:is_toolchain("clang-cl") then
            content = content..[[add_cxflags("-ffast-math")]]
        elseif package:is_toolchain("gcc") or package:is_toolchain("icc") then
            content = content..[[add_cxflags("-ffast-math")]]
        end
        io.writefile("xmake.lua", content)

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)

    on_test(function (package)
        assert(package:has_cfuncs("yyjson_read", {includes = "yyjson/yyjson.h"}))
    end)