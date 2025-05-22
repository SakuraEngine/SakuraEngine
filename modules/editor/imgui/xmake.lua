shared_module("SkrImGui", "SKR_IMGUI")
    -- misc
    skr_unity_build()

    -- deps
    add_deps("SDL3", {private=true})
    public_dependency("SkrInput")
    public_dependency("SkrRenderGraph")

    -- includes
    add_includedirs("include", {public=true})
    add_files("src/**.cpp")

    -- font files
    skr_install_rule()
    skr_install("download", {
        name = "SourceSansPro-Regular.ttf",
        install_func = "file",
        out_dir = "resources/font"
    })

    -- compile shader
    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders",
        dxil_outdir = "/../resources/shaders"
    })
    add_files("shaders/*.hlsl")

    -- ImGui
    --! use version with dynamic fonts, maybe unstable
    --! Branch: features/dynamic_fonts
    --! Commit: cde94b1 - WIP - Fonts: fixed broken support for legacy backend due to a mismatch with initial pre-build baked id.
    --! CommitSHA: cde94b14f3d26506d324fc2826bd92b463dd4248
    add_includedirs("imgui/include", {public = true})
    add_includedirs("imgui/src", {private = true})
    add_files("imgui/src/**.cpp")
    add_defines("IMGUI_DEFINE_MATH_OPERATORS")