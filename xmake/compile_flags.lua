target("SkrCompileFlags")
    set_group("00.utilities")
    set_kind("headeronly")

    set_warnings("all", "extra")

    -- uses utf-8 charset at runtime
    add_cxflags(
        "/execution-charset:utf-8", 
        "/source-charset:utf-8",
        {public = true, tools = {"clang_cl", "cl"}})

    -- disable c++ warnings for gcc/clang
    add_cxflags(
        "-Wno-unused-private-field", 
        "-Wno-deprecated-builtins",
        "-Wno-pragma-system-header-outside-header",
        "-Wno-ambiguous-reversed-operator",
        "-Wno-unused-command-line-argument",
        "-Wno-format", 
        "-Wno-switch",
        "-Wno-misleading-indentation",
        "-Wno-unknown-pragmas",
        "-Wno-unused-function",
        "-Wno-ignored-attributes", 
        -- "-Wno-deprecated-declarations", -- will disable SKR_DEPRECATED
        "-Wno-nullability-completeness", 
        "-Wno-tautological-undefined-compare",
        "-Werror=return-type",
        "-Wno-unused-parameter", -- too much check nned to disable it
        "-Wno-sign-compare", -- too much check nned to disable it
        "-Wno-ignored-qualifiers", -- const int func()
        "-Wno-deprecated-copy-with-user-provided-copy", -- usally trigger it manually
        {public = true, tools = {"gcc", "clang_cl", "clang"}}
    )
    -- disable c warnings for gcc/clang
    add_cflags(
        "-Wno-unused-variable", -- we not care about unused variable in C
        {public = true, tools={"gcc", "clang", "clang_cl"}}
    )

    -- disable c++ warnings for msvc
    add_cxflags(
        "/wd4251", -- 'type' : class 'type1' needs to have dll-interface to be used by clients of class 'type2'
        "/wd4100", -- Wno-unused-parameter
        "/wd4018", -- Wno-sign-compare
        "/wd4389", -- Wno-sign-compare ==
        "/wd4245", -- = lost data for signed -> unsigned
        "/wd4244", -- = lost data for bigger to smaller
        "/wd4267", -- = lost data for bigger to smaller
        "/wd4127", -- conditional expression is constant
        "/wd4706", -- assignment within conditional expression
        "/wd4458", -- declaration of 'xxxx' hides class member
        "/wd4275", -- non dll-interface struct 'xxxx' used as base for dll-interface struct 'xxxx'
        "/wd4201", -- nonstandard extension used: nameless struct/union
        "/wd4624", -- destructor was implicitly defined as deleted
        -- "/wd4819", -- The file contains a character that cannot be represented in the current code page (936). Save the file in Unicode format to prevent data loss
        "/wd4456", -- declaration of 'xxxx' hides previous local declaration
        "/wd4457", -- declaration of 'xxxx' hides function parameter
        "/wd4459", -- declaration of 'xxxx' hides global declaration
        "/wd4324", -- structure was padded due to alignment specifier
        "/wd4702", -- unreachable code
        {public = true, tools={"cl"}}
    )
    -- disable c warnings for msvc
    add_cflags(
        "/wd4189", -- local variable is initialized but not referenced
        {public = true, tools={"cl"}}
    )

    -- util flag for cl
    add_cxflags(
        "/Zc:__cplusplus", -- enable __cplusplus macro
        "/FC", -- output full path in diagnostics
        "/GR-", -- disable RTTI
        { public = true, tools = {"clang_cl", "cl"} }
    )

    -- util flag for clang-cl
    add_cxflags(
        "-Wno-clang-cl-pch",
        "-Wno-microsoft-cast", -- microsoft cast extension
        "-Wno-microsoft-include", -- microsoft include extension
        "-Wno-microsoft-enum-forward-reference", -- microsoft enum forward declaration extension
        -- "/Zc:dllexportInlines-", -- strip inline function from dll export
        {public = true, tools = {"clang_cl"}}
    )

    -- cl link flags
    add_ldflags(
        "/IGNORE:4217,4286", -- dllexport warning
        {public = true, tools = {"clang_cl", "cl"}}
    )

    -- asan link flags
    if is_mode("asan") then
        add_ldflags("-fsanitize=address", { public = true, tools = "clang_cl" })
        add_ldflags("/fsanitize=address", { public = true, tools = "cl" })
    end

