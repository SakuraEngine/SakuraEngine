ftl_includes_dir = "$(projectdir)/thirdparty/FiberTaskingLib/include"
boost_context_include_dir = "$(projectdir)/thirdparty/boost_context/include"

table.insert(include_dir_list, boost_context_include_dir)
table.insert(include_dir_list, ftl_includes_dir)

option("ftl_fiber_canary_bytes")
    set_default(false)
    set_showmenu(true)
    set_description("Enable canary bytes in fiber switching logic, to help debug errors")
option_end()