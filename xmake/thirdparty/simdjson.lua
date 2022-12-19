simdjson_include_dir = "$(projectdir)/thirdparty/simdjson/include"
simdjson_source_dir = "$(projectdir)/thirdparty/simdjson/source"

table.insert(include_dir_list, simdjson_include_dir)

target("simdjson")
    set_group("00.thirdparty")
    add_defines("SIMDJSON_IMPLEMENTATION_HASWELL=0")
    add_defines("SIMDJSON_AVX512_ALLOWED=0")
    add_includedirs(simdjson_include_dir)
    add_cxflags(project_cxflags, {public = true})
    add_defines(defs_list, {public = true})
    if (is_os("windows") and not is_mode("asan")) then 
        set_kind("headeronly")    
        if (is_mode("release")) then
            add_links(sdk_libs_dir.."simdjson", {public=true} )
        else
            add_links(sdk_libs_dir.."simdjsond", {public=true} )
        end
    else
        set_kind("static")    
        add_files(simdjson_source_dir.."/simdjson.cpp")
    end