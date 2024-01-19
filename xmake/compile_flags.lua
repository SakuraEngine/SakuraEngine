target("SkrCompileFlags")
    set_group("00.utilities")
    set_kind("headeronly")
    -- uses utf-8 charset at runtime
    if is_plat("windows") then
        add_cxflags("/execution-charset:utf-8", "/source-charset:utf-8")
    end
    add_cxflags("-Wno-unused-private-field", "-Wno-deprecated-builtins",
        "-Wno-pragma-system-header-outside-header",
        "-Wno-ambiguous-reversed-operator",
        "-Wno-unused-command-line-argument",
        "-Wno-format", "-Wno-switch",
        "-Wno-misleading-indentation",
        "-Wno-unknown-pragmas", "-Wno-unused-function",
        "-Wno-ignored-attributes", "-Wno-deprecated-declarations",
        "-Wno-nullability-completeness", "-Wno-tautological-undefined-compare",
        "-Werror=return-type", { public = true, tools = {"clang_cl", "clang"} }
    )
    add_cxflags("/Zc:__cplusplus", "/FC", "/GR-", "/wd4251",
        { public = true, tools = {"clang_cl", "cl"} }
    )
    add_cxflags("-Wno-microsoft-cast", 
        "-Wno-microsoft-include", 
        "-Wno-microsoft-enum-forward-reference",
        { public = true, tools = {"clang_cl"} }
    )
    if is_mode("asan") then
        add_ldflags("-fsanitize=address", { public = true, tools = "clang_cl" })
        add_ldflags("/fsanitize=address", { public = true, tools = "cl" })
    end
    add_ldflags("/IGNORE:4217,4286", { public = true, tools = {"clang_cl", "cl"} })

