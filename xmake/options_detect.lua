option("is_clang")
    add_cxxsnippets("is_clang", 'if(__clang__) return 0;', {tryrun = true})
option_end()
option("is_msvc")
    add_cxxsnippets("is_msvc", 'if(_MSC_VER) return 0;', {tryrun = true})
option_end()
option("is_unix")
    add_cxxsnippets("is_unix", 'if(__unix__) return 0;', {tryrun = true})
option_end()
option("pointer-size")
    add_cxxsnippets("VOID_P_SIZE", 'printf("%d", sizeof(void*)); return 0;', {output = true, number = true})
option_end()

project_cxflags = {}
project_mxflags = {}
if(has_config("is_clang")) then
    table.insert(project_cxflags, "-Wno-format")
    table.insert(project_cxflags, "-Wno-switch")
    table.insert(project_cxflags, "-Wno-unknown-pragmas")
    table.insert(project_cxflags, "-Wno-ignored-attributes")
    table.insert(project_cxflags, "-Wno-ignored-attributes")
    table.insert(project_cxflags, "-Wno-deprecated-declarations")
    table.insert(project_cxflags, "-Wno-nullability-completeness")
end