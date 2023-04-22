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

project_ldflags = {}
project_cxflags = {}
project_mxflags = {}

-- uses utf-8 charset at runtime
if is_host("windows") then
    table.insert(project_cxflags, "/execution-charset:utf-8")
    table.insert(project_cxflags, "/source-charset:utf-8")
end

if(has_config("is_clang")) then
    table.insert(project_cxflags, "-Wno-unused-command-line-argument")
    table.insert(project_cxflags, "-Wno-format")
    -- table.insert(project_cxflags, "-Wno-deprecated-builtins")
    table.insert(project_cxflags, "-Wno-switch")
    table.insert(project_cxflags, "-Wno-misleading-indentation")
    table.insert(project_cxflags, "-Wno-unknown-pragmas")
    table.insert(project_cxflags, "-Wno-unused-function")
    table.insert(project_cxflags, "-Wno-ignored-attributes")
    table.insert(project_cxflags, "-Wno-deprecated-declarations")
    table.insert(project_cxflags, "-Wno-nullability-completeness")
    table.insert(project_cxflags, "-Wno-tautological-undefined-compare")
    table.insert(project_cxflags, "-Werror=return-type")
    -- enable time trace with clang compiler
    table.insert(project_cxflags, "-ftime-trace")
    if(has_config("is_msvc")) then
        table.insert(project_cxflags, "-Wno-microsoft-cast")
        table.insert(project_cxflags, "-Wno-microsoft-enum-forward-reference")
        if (is_mode("asan")) then
            table.insert(project_ldflags, "-fsanitize=address")
        end
    end
end

if(has_config("is_msvc")) then
    table.insert(project_cxflags, "/Zc:__cplusplus")
    table.insert(project_cxflags, "/FC")
    table.insert(project_cxflags, "/GR-")
    table.insert(project_cxflags, "/wd4251")
    if (is_mode("asan")) then
        table.insert(project_ldflags, "/fsanitize=address")
    end
end