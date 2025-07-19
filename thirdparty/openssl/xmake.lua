target("openssl")
    set_kind("headeronly")

    add_includedirs("include", {public = true})
    
    -- download prebuilt binaries
    skr_install_rule()
    skr_install("download", {
        name = "openssl_3.5.0",
        install_func = "sdk",
    })

    -- add add_links
    add_linkdirs("$(builddir)/$(os)/$(arch)/$(mode)", {public = true})
    add_links("libcrypto", {public = true})
    add_links("libssl", {public = true})