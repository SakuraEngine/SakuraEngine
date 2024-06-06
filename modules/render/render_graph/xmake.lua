shared_module("SkrRenderGraph", "SKR_RENDER_GRAPH", engine_version)
    public_dependency("SkrRT", engine_version)
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_includedirs("include", {public=true})
    -- add_files("src/graphviz/*.cpp", {unity_ignored = true})
    add_files("src/frontend/*.cpp", "src/graphviz/*.cpp")
    add_files("src/backend/*.cpp", {unity_group = "backend"})
    add_files("src/phases/*.cpp", {unity_group = "backend"})    

shared_pch("SkrRenderGraph")
    add_files("include/**.hpp")

private_pch("SkrRenderGraph")
    add_files("src/pch.hpp")