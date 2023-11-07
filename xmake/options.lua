option("build_tools")
    set_default(true)
    set_description("Toggle to build tools of SakuraRuntime")
option_end()

option("build_samples")
    set_default(true)
    set_description("Toggle to build samples of SakuraRuntime")
option_end()

option("build_wasm3_samples")
    set_default(false)
    set_description("Toggle to build samples of wasm3")
option_end()

option("build_cgpu_samples")
    set_default(false)
    set_description("Toggle to build samples of CGPU")
option_end()

option("build_rg_samples")
    set_default(false)
    set_description("Toggle to build samples of render graph")
option_end()

option("build_editors")
    set_default(false)
    set_description("Toggle to build editors of SakuraRuntime")
option_end()

option("build_tests")
    set_default(false)
    set_description("Toggle to build tests of SakuraRuntime")
option_end()

option("build_AAA")
    set_default(false)
    set_description("Toggle to build AAA project")
option_end()

option("use_profile")
    -- "auto", "enable", "disable"
    set_default("auto")
    set_values("auto", "enable", "disable")
    set_description("Toggle to use tracy profile")
option_end()

option("cxx_version")
    -- "cxx20", "cxx17", "cxx23"
    set_default("cxx20")
    set_values("cxx20", "cxx17", "cxx23")
    set_description("c++ version of project")
option_end()

option("c_version")
    -- "c11"
    set_default("c11")
    set_values("c11")
    set_description("c version of project")
option_end()
