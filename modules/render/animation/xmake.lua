shared_module("SkrOzz", "SKR_OZZ", engine_version)
    set_exceptions("no-cxx")
    set_optimize("fastest")
    public_dependency("SkrRT", engine_version)
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_includedirs("ozz", {public=true})
    add_includedirs("ozz_src", {public=false})
    add_files("ozz_src/**.cc")
        
private_pch("SkrOzz")
    add_files("ozz/SkrAnim/ozz/*.h")

shared_pch("SkrOzz")
    add_files("ozz/SkrAnim/ozz/*.h")
    
---------------------------------------------------------------------------------------

codegen_component("SkrAnim", { api = "SKR_ANIM", rootdir = "include/SkrAnim" })
    add_files("include/**.h")
    add_files("include/**.hpp")

shared_module("SkrAnim", "SKR_ANIM", engine_version)
    public_dependency("SkrOzz", engine_version)
    public_dependency("SkrRenderer", engine_version)
    add_includedirs("include", {public=true})
    add_includedirs("ozz", {public=true})
    add_rules("c++.unity_build", {batchsize = default_unity_batch})
    add_files("src/**.cpp")

private_pch("SkrAnim")
    add_files("src/pch.hpp")

shared_pch("SkrAnim")
    add_files("include/**.h")
    add_files("include/**.hpp")