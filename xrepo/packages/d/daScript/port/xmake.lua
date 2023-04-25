add_requires("daScriptCore 2023.4.25-skr.1")

includes("rules/AOT.lua")

target("daStandard")
    set_kind("shared")
    set_languages("c++17")
    add_packages("daScriptCore")
    add_rules("AOT")
    add_files("daslib/**.cpp")
    add_files(
        "daslib/functional.das",
        "daslib/json.das",
        "daslib/json_boost.das",
        "daslib/regex.das",
        "daslib/regex_boost.das",
        "daslib/strings_boost.das",
        "daslib/random.das",
        "daslib/math_boost.das",
        "daslib/utf8_utils.das",

        "daslib/faker.das",
        "daslib/fuzzer.das"
    )