add_requires("daScriptCore 2023.4.25-skr.10")
add_requires("daScriptTool 2023.4.25-skr.10")

includes("rules/AOT.lua")

target("daStandard")
    set_kind("static")
    set_languages("c++17")
    add_packages("daScriptCore")
    add_packages("daScriptTool")
    add_rules("AOT")
    add_files("daslib/**.cpp")
    add_files(
        "daslib/algorithm.das",
        "daslib/archive.das",
        "daslib/apply.das",
        "daslib/array_boost.das",
        "daslib/assert_once.das",

        "daslib/base64.das",

        "daslib/constant_expression.das",
        "daslib/contracts.das",
        "daslib/coroutines.das",

        "daslib/defer.das",

        "daslib/enum_trait.das",

        "daslib/functional.das",
        "daslib/faker.das",
        "daslib/fuzzer.das",

        "daslib/interfaces.das",
        "daslib/is_local.das",

        "daslib/jobque_boost.das",
        "daslib/json.das",
        "daslib/json_boost.das",

        "daslib/math_boost.das",

        "daslib/random.das",
        "daslib/regex.das",
        "daslib/regex_boost.das",

        "daslib/strings_boost.das",
        "daslib/sort_boost.das",

        "daslib/utf8_utils.das"
    )