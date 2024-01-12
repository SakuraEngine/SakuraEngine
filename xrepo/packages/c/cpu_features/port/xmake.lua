local cpu_features_include_dir = "cpu_features/include"

set_languages("c11")

target("cpu_features")
    set_kind("static")
    set_optimize("fastest")
    add_headerfiles(cpu_features_include_dir.."/(**.h)")
    add_includedirs(cpu_features_include_dir, {public = true})
    add_includedirs(cpu_features_include_dir.."/cpu_features", {public = false})
    add_files("cpu_features/build.cpu_features.c")