ftl_includes_dir = "$(projectdir)/thirdparty/FiberTaskingLib/include"

target("SkrCompileFlags")
    add_includedirs(ftl_includes_dir, {public = true})

option("ftl_fiber_canary_bytes")
    set_default(false)
    set_showmenu(true)
    set_description("Enable canary bytes in fiber switching logic, to help debug errors")
option_end()