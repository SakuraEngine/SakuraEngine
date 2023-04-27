if is_os("windows") or is_os("macosx") or is_os("linux") then
    if not is_mode("one_archive") then
        add_requires("efsw")
        efsw_pak = true
    end
end

shared_module("SkrTweak", "SKR_TWEAK", engine_version)
    set_group("01.modules")
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_defines("SKR_SOURCE_ROOT=R\"($(projectdir))\"", {public=false})
    if(efsw_pak) then
        add_packages("efsw")
    end
