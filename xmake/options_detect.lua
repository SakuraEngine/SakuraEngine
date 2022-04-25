option("is_clang")
    add_cxxsnippets("is_clang", 'if(__clang__) return 0;', {tryrun = true})
option("is_msvc")
    add_cxxsnippets("is_msvc", 'if(_MSC_VER) return 0;', {tryrun = true})
option("is_unix")
    add_cxxsnippets("is_unix", 'if(__unix__) return 0;', {tryrun = true})


project_cxflags = {}
project_mxflags = {}
if(has_config("is_clang")) then
    table.insert(project_cxflags, "-Wno-nullability-completeness")
    table.insert(project_cxflags, "-Wno-ignored-attributes")
    table.insert(project_mxflags, "-Wno-nullability-completeness")
    table.insert(project_mxflags, "-Wno-ignored-attributes")
end