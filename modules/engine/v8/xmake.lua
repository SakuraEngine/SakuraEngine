if is_mode("release") then 
    add_requires("v8 11.2-skr")
elseif is_mode("debug") then 
    add_requires("v8 11.2-skr", {debug=true})
elseif is_mode("releasedbg") then 
    add_requires("v8 11.2-skr", {configs={symbols=true}})
end


shared_module("SkrV8", "SKR_V8", engine_version)
    -- dependencies
    public_dependency("SkrRT", engine_version)
    add_packages("v8", {public = true})

    -- add source files
    add_includedirs("include", {public = true})
    add_files("src/**.cpp")