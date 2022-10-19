target("SkrGAInput")
    set_group("01.modules")
    add_rules("skr.module", {api = "GAINPUT_LIB", version = engine_version})
    add_rules("c++.noexception")
    -- add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    public_dependency("SkrRT", engine_version)
    add_defines("GAINPUT_DEV", {public = true})
    add_defines("GAINPUT_LIB_IMPL", {public = true}) --??WTF?
    add_includedirs("include", {public=true})
    -- gainput
    add_files("src/**.cpp", "src/hidapi/build.0.c")
    if (is_os("macosx")) then 
        add_files("src/**.mm")
        add_mxflags("-fno-objc-arc", {force = true})
        add_frameworks("CoreFoundation", "Cocoa", "IOKit", {public = true})
    end