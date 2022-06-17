target("SkrGAInput")
    set_kind("shared")
    add_deps("SkrRT")
    add_includedirs("include", {public=true})
    add_defines("GAINPUT_LIB_DYNAMIC_USE", {public=true})
    add_defines("GAINPUT_LIB_DYNAMIC")
    -- gainput
    add_files("src/**.cpp", "src/hidapi/build.0.c")
    if (is_os("macosx")) then 
        add_files("src/**.m", "src/**.mm")
    end