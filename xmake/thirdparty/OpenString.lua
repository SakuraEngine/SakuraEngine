ostring_include_dir = "$(projectdir)/thirdparty/OpenString/include"
ostring_private_include_dir = "$(projectdir)/thirdparty/OpenString/include/OpenString"
ostring_source_dir = "$(projectdir)/thirdparty/OpenString/source"

target("SkrCompileFlags")
    add_defines(
        "OPEN_STRING_API=SKR_RUNTIME_API"
    , {public = true})

target("SkrRoot")
    add_includedirs(ostring_include_dir, {public = true})

target("SkrRTStatic")
    add_includedirs(ostring_private_include_dir)

target("SkrRT")
    add_files(ostring_source_dir.."/build.*.cpp")
    add_includedirs(ostring_private_include_dir)