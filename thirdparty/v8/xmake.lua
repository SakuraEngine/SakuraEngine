target("v8")
    set_kind("headeronly")

    add_includedirs("include", { public = true })
    add_linkdirs("$(builddir)/$(os)/$(arch)/$(mode)", {public=true})

    -- v8 sys env
    if is_plat("linux", "bsd") then
        add_syslinks("pthread", "dl")
    elseif is_plat("windows") then
        add_syslinks("user32", "winmm", "advapi32", "dbghelp", "shlwapi")
    end

    -- v8 def
    -- add_defines("USING_V8_PLATFORM_SHARED=1", { public = true })
    -- add_defines("USING_V8_SHARED=1", { public = true })

    -- v8 link
    if is_plat("windows") then
        add_links(
            "v8.dll",
            "v8_libplatform.dll",
            { public = true }
        )
    elseif is_plat("linux", "macosx") then
        add_links(
            "libv8",
            "libv8_libbase",
            "libv8_libplatform",
            "libthird_party_zlib",
            { public = true }
        )
    end

    -- packages
    local script_dir = os.scriptdir()
    local install_func = function (installer)
        -- installer.print_log = true
        -- installer:cp("include/**.h", path.join(script_dir, "include"), { 
        --     rootdir = "include",
        --     dst_is_dir = true,
        --     rm_dst = true,
        -- })
        installer:bin("**")
    end
    skr_install_rule()
    -- FIXME. v8 clang-cl use /Zc:dllexportInlines- flag, which cause symbol lost in dll, so hear use msvc version directly
    -- skr_install("download", {
    --     name = "v8_11.2_clang_cl",
    --     install_func = "custom",
    --     custom_install = install_func,
    --     toolchain = {"clang-cl"},
    -- })
    skr_install("download", {
        name = "v8_11.8.172",
        install_func = "custom",
        custom_install = install_func,
        -- debug = not is_mode("release"),
        -- toolchain = {"msvc"},
})