rule("c++.exception")
    on_config(function (target)
        if (target:has_tool("cxx", "cl")) then
            target:add("cxflags", "/EHsc", {force = true})
            target:add("cxxflags", "/EHsc", {force = true})
            target:add("defines", "_HAS_EXCEPTIONS=1", {force = true})
        elseif(target:has_tool("cxx", "cl") or target:has_tool("cxx", "clang-cl")) then
            target:add("cxflags", "-fexceptions", {force = true})
            target:add("cxflags", "-fcxx-exceptions", {force = true})
        end
    end)

rule("c++.noexception")
    on_config(function (target)
        if (target:has_tool("cxx", "cl")) then
            target:add("cxflags", "/EHs-c-", {force = true})
            target:add("cxflags", "/D_HAS_EXCEPTIONS=0", {force = true})
            target:add("defines", "_HAS_EXCEPTIONS=0", {force = true})
        elseif(target:has_tool("cxx", "cl") or target:has_tool("cxx", "clang-cl")) then
            target:add("cxflags", "-fno-exceptions", {force = true})
            target:add("cxflags", "-fno-cxx-exceptions", {force = true})
        end
    end)