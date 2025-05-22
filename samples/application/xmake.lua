includes("ogui2/xmake.lua")

-- includes("game/xmake.lua") -- FIXME. lua support

if os.host() == "windows" and false then
    includes("chat/xmake.lua")
end

executable_module("VMemController", "VMEM_CONTROLLER")
    set_group("04.examples/application")
    public_dependency("SkrRenderGraph")
    public_dependency("SkrImGui")
    set_exceptions("no-cxx")
    add_deps("AppSampleCommon")
    skr_unity_build()
    add_files("vmem_controller/**.cpp")

executable_module("Live2DViewer", "LIVE2D_VIEWER")
    set_group("04.examples/application")
    public_dependency("SkrLive2D")
    public_dependency("SkrImGui")

    -- install
    skr_install_rule()
    skr_install("files", {
        name = "live2d-resources",
        files = {
            "live2d-viewer/**.json",
            "live2d-viewer/**.moc3",
            "live2d-viewer/**.png",
        },
        out_dir = "../resources/Live2DViewer",
        root_dir = "live2d-viewer/resources/",
    })
    skr_install("download", {
        name = "SourceSansPro-Regular.ttf",
        install_func = "file",
        out_dir = "../resources/font"
    })

    add_rules("utils.dxc", {
        spv_outdir = "/../resources/shaders/Live2DViewer",
        dxil_outdir = "/../resources/shaders/Live2DViewer"})
    set_exceptions("no-cxx")
    skr_unity_build()
    add_deps("AppSampleCommon")
    add_includedirs("live2d-viewer/include", {public=true})
    add_files("live2d-viewer/src/main.cpp", "live2d-viewer/src/viewer_module.cpp")
    -- add_files("live2d-viewer/shaders/**.hlsl")

--[[
    if(not has_config("shipping_one_archive")) then

    shared_module("GameTool", "GAMETOOL")
        set_group("04.examples/application")
        public_dependency("SkrToolCore", "0.1.0")
        public_dependency("GameRuntime", "0.1.0")
        add_includedirs("gametool/include", {public=true})
        add_deps("AppSampleCommon")
        add_files("gametool/src/**.cpp")
        skr_unity_build()
        on_config(function (target, opt)
            local dep = target:dep("GameRuntime");
            local toolgendir = path.join(dep:autogendir({root = true}), dep:plat(), "codegen", dep:name(), "tool")
            if os.exists(toolgendir) then
                target:add("includedirs", toolgendir)
                local cppfiles = os.files(path.join(toolgendir, "/*.cpp"))
                for _, file in ipairs(cppfiles) do
                    target:add("files", file)
                end
            end
        end)

    end
]]--
