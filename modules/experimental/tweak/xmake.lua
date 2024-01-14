if is_os("windows") or is_os("macosx") or is_os("linux") then
    if(not has_config("shipping_one_archive")) then
        add_requires("efsw")
        efsw_pak = true
    end
end

shared_module("SkrTweak", "SKR_TWEAK", engine_version)
    public_dependency("SkrRT", engine_version)
    add_includedirs("include", {public=true})
    add_rules("c++.unity_build", {batchsize = default_unity_batch_size})
    add_files("src/**.cpp")
    add_defines("SKR_SOURCE_ROOT=R\"($(projectdir))\"", {public=false})
    if(efsw_pak) then
        add_packages("efsw")
    end
