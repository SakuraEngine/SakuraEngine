project_ldflags = {}

target("SkrCompileFlags")
    set_kind("headeronly")
    if (is_os("macosx") or is_os("linux")) then
        on_load(function(target)
            cxflags = target:get("cxflags")
            target:add("mxflags", cxflags, {public = true, force = true})
        end)
    end
    add_cxflags(
        "-Wno-ambiguous-reversed-operator", 
        "-Wno-unused-command-line-argument",
        "-Wno-format",
        "-Wno-deprecated-builtins",
        "-Wno-switch",
        "-Wno-misleading-indentation",
        "-Wno-unknown-pragmas",
        "-Wno-unused-function",
        "-Wno-ignored-attributes",
        "-Wno-deprecated-declarations",
        "-Wno-nullability-completeness",
        "-Wno-tautological-undefined-compare",
        "-Werror=return-type",
        "-ftime-trace",
    {public = true, force = true, tools = {"clang_cl", "clang"}})
    add_cxflags(
        "clang_cl::-Wno-microsoft-cast",
        "clang_cl::-Wno-microsoft-enum-forward-reference",
    {public = true, force = true})
    if (is_mode("asan")) then
        add_cxflags(
            "cl::/fsanitize=address",
            "clang_cl::-fsanitize=address",
        {public = true, force = true})
    end
    -- uses utf-8 charset at runtime
    if is_host("windows") then
        add_cxflags(
            "/execution-charset:utf-8",
            "/source-charset:utf-8",
            {public = true, force = true})
    end
    add_cxflags(
        "/IGNORE:4217,4286", 
        "/Zc:__cplusplus",
        "/FC",
        "/GR-",
        "/wd4251",
    {public = true, force = true, tools = {"clang_cl", "cl"}})