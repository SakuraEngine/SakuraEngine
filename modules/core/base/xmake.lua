static_component("SkrBase", "SkrCore")
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_deps("SkrProfile", {public = true})
    add_deps("SkrCompileFlags", {public = true})
    add_includedirs("include", {public = true})
    add_files("src/**/build.*.c", "src/**/build.*.cpp")
    -- for guid/uuid
    if (is_os("windows")) then 
        add_syslinks("Ole32", {public = true})
    end
    if (is_os("macosx")) then 
        add_frameworks("CoreFoundation", {public = true})
    end