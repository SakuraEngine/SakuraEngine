target("rg-deferred")
    set_kind("binary")
    add_deps("SkrRT")
    add_files("rg-deferred/*.cpp")
    
target("cgpu-texture")
    set_kind("binary")
    add_deps("SkrRT")
    add_files("cgpu-texture/*.c")