package("ispc")
    set_kind("toolchain")
    set_homepage("https://ispc.github.io/")
    set_description("Intel® Implicit SPMD Program Compiler")
    set_license("BSD-3-Clause")

    if is_host("windows") then
        add_urls("https://github.com/ispc/ispc/releases/download/v$(version)/ispc-v$(version)-windows.zip")
    elseif is_host("macosx") then
        if is_arch("arm64") then
            add_urls("https://github.com/ispc/ispc/releases/download/v$(version)/ispc-v$(version)-macOS.arm64.tar.gz")
        else
            add_urls("https://github.com/ispc/ispc/releases/download/v$(version)/ispc-v$(version)-macOS.x86_64.tar.gz")
        end
    elseif is_host("linux") then
        add_urls("https://github.com/ispc/ispc/releases/download/v$(version)/ispc-v$(version)-linux.tar.gz")
    end

    on_install("@windows", "@macosx", "@linux", function (package)
        os.cp(path.join(package:scriptdir(), "rules"), ".")
        os.cp("*", package:installdir())
    end)

    on_test(function (package)
        os.vrun("ispc --version")
    end)