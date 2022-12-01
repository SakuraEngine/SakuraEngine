if(not has_config("shipping_one_archive")) then
    add_requires("efsw")
end
 
shared_module("SkrTweak", "SKR_TWEAK", engine_version)
    set_group("01.modules")
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")
    add_defines("SKR_SOURCE_ROOT=R\"($(projectdir))\"", {public=false})
    if(not has_config("shipping_one_archive")) then
        add_packages("efsw")
    end