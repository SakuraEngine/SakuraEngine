rule("c++.exception")
    on_config(function (target)
        --[[
        if(has_config("is_msvc")) then
            target:add("cxflags", "/EHsc")
            target:add("cxxflags", "/EHsc")
            target:add("defines", "_HAS_EXCEPTIONS=1")
        elseif(has_config("is_clang")) then
            target:add("cxflags", "-fexceptions")
            target:add("cxflags", "-fcxx-exceptions")
        end
        ]]--
    end)

rule("c++.noexception")
    on_config(function (target)
        --[[
        if(has_config("is_msvc")) then
            target:add("cxflags", "/EHs-c-")
            target:add("cxflags", "/D_HAS_EXCEPTIONS=0")
            target:add("defines", "_HAS_EXCEPTIONS=0")
        elseif(has_config("is_clang")) then
            target:add("cxflags", "-fno-exceptions")
            target:add("cxflags", "-fno-cxx-exceptions")
        end
        ]]--
    end)